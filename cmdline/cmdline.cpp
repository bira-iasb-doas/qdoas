#include <cstdio>
#include <cstring>

#include <iostream>
#include <string>

#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QFile>
#include <QString>
#include <QList>
#include <QDir>
#include <QFileInfo>

#include "CWorkSpace.h"
#include "CQdoasConfigHandler.h"
#include "CProjectConfigItem.h"
#include "CBatchEngineController.h"
#include "CEngineResponse.h"
#include "CProjectConfigTreeNode.h"
#include "constants.h"

#include "CConvConfigHandler.h"

#include "mediate_xsconv.h"

#include "QdoasVersion.h"
#include "debugutil.h"

//-------------------------------------------------------------------
// types
//-------------------------------------------------------------------

enum RunMode {
  None,
  Error,
  Help,
  Batch
};

enum BatchTool {
  Unknown,
  Qdoas,
  Convolution,
  Ring,
  Usamp
};

typedef struct commands
{
  QString configFile;
  QString projectName;
  QList<QString> filenames;
  QString outputDir;
} commands_t;

//-------------------------------------------------------------------
// declarations
//-------------------------------------------------------------------

enum RunMode parseCommandLine(int argc, char **argv, commands_t *cmd);
enum BatchTool requiredBatchTool(const QString &filename);
void showUsage();
void showHelp();
int batchProcess(commands_t *cmd);

int batchProcessQdoas(commands_t *cmd);
int readConfigQdoas(commands_t *cmd, QList<const CProjectConfigItem*> &projectItems);
int analyseProjectQdoas(const CProjectConfigItem *projItem,  const QString &outputDir, const QList<QString> &filenames);
int analyseProjectQdoas(const CProjectConfigItem *projItem, const QString &outputDir);
int analyseProjectQdoasPrepare(void **engineContext, const CProjectConfigItem *projItem, const QString &outputDir,
			       CBatchEngineController *controller);
int analyseProjectQdoasFile(void *engineContext, CBatchEngineController *controller, const QString &filename);
int analyseProjectQdoasTreeNode(void *engineContext, CBatchEngineController *controller, const CProjectConfigTreeNode *node);
int analyseProjectQdoasDirectory(void *engineContext, CBatchEngineController *controller, const QString &dir,
				 const QString &filters, bool recursive);

int batchProcessConvolution(commands_t *cmd);
int batchProcessRing(commands_t *cmd);
int batchProcessUsamp(commands_t *cmd);

//-------------------------------------------------------------------

int main(int argc, char **argv)
{
  int retCode = 0;

  if (argc == 1) {

    showUsage();
  }
  else {
    commands_t cmd;

    enum RunMode runMode = parseCommandLine(argc, argv, &cmd);

    switch (runMode) {
    case None:
    case Error:
      showUsage();
      break;
    case Help:
      showHelp();
      break;
    case Batch:
      retCode = batchProcess(&cmd);
      break;
    }
  }

  return retCode;
}

//-------------------------------------------------------------------

enum RunMode parseCommandLine(int argc, char **argv, commands_t *cmd)
{
  // extract data from command line
  enum RunMode runMode = None;
  int i = 1;

  while (runMode != Error && i < argc) {

    // options ...
    if (argv[i][0] == '-') {

      if (!strcmp(argv[i], "-c")) { // configuration file ...
	if (++i < argc && argv[i][0] != '-') {
	  if (cmd->configFile.isEmpty()) {
	    cmd->configFile = argv[i];
	    runMode = Batch;
	  }
	  else
	    std::cout << "Duplicate '-c' option." << std::endl;
	}
	else {
	  runMode = Error;
	  std::cout << "Option '-c' requires an argument (configuration file)." << std::endl;
	}

      }
      else if (!strcmp(argv[i], "-p")) { // project name file ...
	if (++i < argc && argv[i][0] != '-') {
	  cmd->projectName = argv[i];
	}
	else {
	  runMode = Error;
	  std::cout << "Option '-p' requires an argument (project name)." << std::endl;
	}

      }
      else if (!strcmp(argv[i], "-f")) { // filename ...
	if (++i < argc && argv[i][0] != '-') {
	  cmd->filenames.push_back(argv[i]);
	}
	else {
	  runMode = Error;
	  std::cout << "Option '-f' requires an argument (filename)." << std::endl;
	}

      }
      else if (!strcmp(argv[i], "-o")) { // output directory ...
	if (++i < argc && argv[i][0] != '-') {
	  cmd->outputDir = argv[i];
	}
	else {
	  runMode = Error;
	  std::cout << "Option '-o' requires an argument (directory)." << std::endl;
	}

      }
      else if (!strcmp(argv[i], "-h")) { // help ...
	runMode = Help;
      }

    }
    else {
      runMode = Error;
      std::cout << "Invalid argument '" << argv[i] << "'" << std::endl;
    }

    ++i;
  }

  // consistency checks ??

  return runMode;
}

//-------------------------------------------------------------------

int batchProcess(commands_t *cmd)
{
  // determine the tool to use based on the config file ...

  enum BatchTool batchTool = requiredBatchTool(cmd->configFile);

  switch (batchTool) {
  case Qdoas:
    return batchProcessQdoas(cmd);
    break;
  case Convolution:
    return batchProcessConvolution(cmd);
    break;
  case Ring:
    return batchProcessRing(cmd);
    break;
  case Usamp:
    return batchProcessUsamp(cmd);
    break;
  default:
    std::cout << "Failed to determine configuration file type." << std::endl;
  }

  return 1;
}

//-------------------------------------------------------------------

enum BatchTool requiredBatchTool(const QString &filename)
{
  enum BatchTool type = Unknown;

  FILE *fp = fopen(filename.toAscii().constData(), "r");
  if (fp != NULL) {
    char buffer[256];

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
      // should begin with '<?xml'
      if (strlen(buffer) > 5 && !strncmp(buffer, "<?xml", 5)) {

	if (fgets(buffer, sizeof(buffer), fp) != NULL) {
	  // opening tag ...
	  int len = strlen(buffer);

	  if (len >= 7 && !strncmp(buffer, "<qdoas>", 7))
	    type = Qdoas;
	  else if (len >= 13 && !strncmp(buffer, "<convolution>", 13))
	    type = Convolution;
	  else if (len >= 6 && !strncmp(buffer, "<ring>", 6))
	    type = Ring;
	  else if (len >= 7 && !strncmp(buffer, "<usamp>", 7))
	    type = Usamp;

	}
      }
    }

    fclose(fp);
  }

  return type;
}

//-------------------------------------------------------------------

void showUsage()
{
  std::cout << "doas_cl -c <config file> [-p <project name>] [-o <output path>] [-f <file>]..." << std::endl << std::endl;
  std::cout << "    -c <config file>   : A Qdoas, convolution, ring or usamp config file." << std::endl;
  std::cout << "                         The tool to invoke is determined from the type of" << std::endl;
  std::cout << "                         configuration file specified." << std::endl;
}

void showHelp()
{
  std::cout << cQdoasVersionString << std::endl << std::endl;
}

int batchProcessQdoas(commands_t *cmd)
{
  QList<const CProjectConfigItem*> projectItems;

  int retCode = readConfigQdoas(cmd, projectItems);

  if (retCode)
    return retCode;

  // projectItems is now a list of projects to process ... guaranteed to have at least one.

  TRACE("Num Projects = " <<  projectItems.size());

  while (!projectItems.isEmpty() && retCode == 0) {

    if (!cmd->filenames.isEmpty()) {
      // if files were specified on the command-line, then ignore the files in the project.
      if (projectItems.size() == 1) {

	const CProjectConfigItem *p = projectItems.takeFirst();

	retCode = analyseProjectQdoas(p, cmd->outputDir, cmd->filenames);

	delete p;
      }
      else {
	// error ... dont know which project to use ...
      }
    }
    else {
      // all projects ... all files ...
      const CProjectConfigItem *p = projectItems.takeFirst();

      retCode = analyseProjectQdoas(p, cmd->outputDir);

      delete p;
    }
  }

  // just cleanup
  while (!projectItems.isEmpty())
    delete projectItems.takeFirst();

  return retCode;
}

int readConfigQdoas(commands_t *cmd, QList<const CProjectConfigItem*> &projectItems)
{
  // read the configuration file
  TRACE("batchProcessQdoas");

  int retCode = 0;

  QFile *file = new QFile(cmd->configFile);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CQdoasConfigHandler *handler = new CQdoasConfigHandler;
  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);

  if (ok) {

    CWorkSpace *ws = CWorkSpace::instance();

    // sites
    const QList<const CSiteConfigItem*> &siteItems = handler->siteItems();
    QList<const CSiteConfigItem*>::const_iterator siteIt = siteItems.begin();
    while (siteIt != siteItems.end()) {

      ws->createSite((*siteIt)->siteName(), (*siteIt)->abbreviation(),
		     (*siteIt)->longitude(), (*siteIt)->latitude(), (*siteIt)->altitude());
      ++siteIt;
    }

    // symbols
    const QList<const CSymbolConfigItem*> &symbolItems = handler->symbolItems();
    QList<const CSymbolConfigItem*>::const_iterator symIt = symbolItems.begin();
    while (symIt != symbolItems.end()) {

      ws->createSymbol((*symIt)->symbolName(), (*symIt)->symbolDescription());
      ++symIt;
    }

    // projects - dont need to be in the workspace ... just keep the project items
    QList<const CProjectConfigItem*> tmpItems = handler->takeProjectItems();

    // is a specific project required ...
    if (!cmd->projectName.isEmpty()) {
      // select only the matching project and discard the rest ...
      while (!tmpItems.isEmpty()) {
	const CProjectConfigItem *p = tmpItems.takeFirst();

	if (p->name() == cmd->projectName)
	  projectItems.push_back(p);
	else
	  delete p;
      }
    }
    else {
      while (!tmpItems.isEmpty()) {
	projectItems.push_back(tmpItems.takeFirst());
      }
    }

    // are there any projects in the result list (projectItems).
    if (projectItems.isEmpty())
      retCode = 1;

  }
  else {
    std::cout << handler->messages().toStdString() << std::endl;
    retCode = 1;
  }
  delete handler;
  delete source;
  delete file;

  return retCode;
}


int analyseProjectQdoas(const CProjectConfigItem *projItem,  const QString &outputDir, const QList<QString> &filenames)
{
  TRACE("analyseProjectQdoas(p, files)");

  void *engineContext;
  int retCode;

  CBatchEngineController *controller = new CBatchEngineController;

  retCode = analyseProjectQdoasPrepare(&engineContext, projItem, outputDir, controller);

  if (retCode)
    return retCode;

  // loop over files ...
  QList<QString>::const_iterator it = filenames.begin();
  while (it != filenames.end()) {

    retCode = analyseProjectQdoasFile(engineContext, controller, *it);

    ++it;
  }

  // destroy engine
  CEngineResponseMessage *msgResp = new CEngineResponseMessage;

  if (mediateRequestDestroyEngineContext(engineContext, msgResp) != 0) {
    msgResp->process(controller);
    retCode = 1;
  }

  delete msgResp;

  return retCode;
}

int analyseProjectQdoas(const CProjectConfigItem *projItem, const QString &outputDir)
{
  TRACE("analyseProjectQdoas(p)");

  void *engineContext;
  int retCode;

  CBatchEngineController *controller = new CBatchEngineController;

  retCode = analyseProjectQdoasPrepare(&engineContext, projItem, outputDir, controller);

  if (retCode)
    return retCode;

  // recursive walk of the files in the config

  retCode = analyseProjectQdoasTreeNode(engineContext, controller, projItem->rootNode());

  // destroy engine
  CEngineResponseMessage *msgResp = new CEngineResponseMessage;

  if (mediateRequestDestroyEngineContext(engineContext, msgResp) != 0) {
    msgResp->process(controller);
    retCode = 1;
  }

  delete msgResp;

  return retCode;
}

int analyseProjectQdoasPrepare(void **engineContext, const CProjectConfigItem *projItem, const QString &outputDir,
			       CBatchEngineController *controller)
{
  TRACE("analyseProjectQdoasPrepare");

  int retCode = 0;
  CEngineResponseMessage *msgResp = new CEngineResponseMessage;

  // copy the project data and mask out any display flags (do not want
  // the engine to create and return visualization data)

  mediate_project_t projectData = *(projItem->properties()); // blot copy
  // TODO projectData.display.

  if (!outputDir.isEmpty() && outputDir.size() < FILENAME_BUFFER_LENGTH-1) {
    // override the output directory
    strcpy(projectData.output.path, outputDir.toAscii().data());
  }

  // create engine
  if (mediateRequestCreateEngineContext(engineContext, msgResp) != 0) {
    msgResp->process(controller);
    delete msgResp;
    return 1;
  }

  // set project
  if (!retCode && mediateRequestSetProject(*engineContext, &projectData, THREAD_TYPE_ANALYSIS, msgResp) != 0) {
    msgResp->process(controller);
    delete msgResp;
    msgResp = new CEngineResponseMessage;
    retCode = 1;
  }

  // set analysis windows
  if (!retCode) {
    const QList<const CAnalysisWindowConfigItem*> awList = projItem->analysisWindowItems();
    int nWindows = awList.size();
    mediate_analysis_window_t *awDataList = new mediate_analysis_window_t[nWindows];
    mediate_analysis_window_t *awCursor = awDataList;

    QList<const CAnalysisWindowConfigItem*>::const_iterator awIt = awList.begin();
    while (awIt != awList.end()) {
      *awCursor = *((*awIt)->properties());
      // mask any display flags ...
      ++awCursor;
      ++awIt;
    }

    if (mediateRequestSetAnalysisWindows(*engineContext, nWindows, awDataList, THREAD_TYPE_ANALYSIS, msgResp) != 0) {
      msgResp->process(controller);
      delete msgResp;
      msgResp = new CEngineResponseMessage;
      retCode = 1;
    }
  }

  if (retCode) {
    // cleanup ... destroy the engine
    if (mediateRequestDestroyEngineContext(*engineContext, msgResp) != 0) {
      msgResp->process(controller);
      retCode = 1;
    }

    *engineContext = NULL;
  }

  return retCode;
}

int analyseProjectQdoasFile(void *engineContext, CBatchEngineController *controller, const QString &filename)
{
  int retCode = 0;
  int result;

  TRACE("analyseProjectQdoasFile " << filename.toStdString());

  CEngineResponseBeginAccessFile *beginFileResp = new CEngineResponseBeginAccessFile(filename);

  result = mediateRequestBeginAnalyseSpectra(engineContext, filename.toAscii().constData(), beginFileResp);
  beginFileResp->setNumberOfRecords(result);

  beginFileResp->process(controller);
  delete beginFileResp;

  if (result == -1)
    return 1;

  // loop based on the controller ...
  while (!retCode && controller->active()) {

    CEngineResponseAccessRecord *resp = new CEngineResponseAccessRecord;

    result = mediateRequestNextMatchingAnalyseSpectrum(engineContext, resp);
    resp->setRecordNumber(result);

    TRACE("   record : " << result);

    if (result == -1)
      retCode = 1;

    resp->process(controller);
    delete resp;
  }

  TRACE("   end file " << retCode);

  return retCode;
}

int analyseProjectQdoasTreeNode(void *engineContext, CBatchEngineController *controller, const CProjectConfigTreeNode *node)
{
  int retCode = 0;

  while (!retCode && node != NULL) {

    TRACE("analyseProjectQdoasTreeNode : " << node->name().toStdString());

    if (node->isEnabled()) {
      switch (node->type()) {
      case CProjectConfigTreeNode::eFile:
	retCode = analyseProjectQdoasFile(engineContext, controller, node->name());
	break;
      case CProjectConfigTreeNode::eFolder:
	retCode = analyseProjectQdoasTreeNode(engineContext, controller, node->firstChild());
	break;
      case CProjectConfigTreeNode::eDirectory:
	retCode = analyseProjectQdoasDirectory(engineContext, controller, node->name(), node->filter(), node->recursive());
	break;
      }
    }

    node = node->nextSibling();
  }

  return retCode;
}


int batchProcessConvolution(commands_t *cmd)
{
  TRACE("batchProcessConvolution");

  int retCode = 0;

  QFile *file = new QFile(cmd->configFile);

  // parse the file
  QXmlSimpleReader xmlReader;
  QXmlInputSource *source = new QXmlInputSource(file);

  CConvConfigHandler *handler = new CConvConfigHandler;
  xmlReader.setContentHandler(handler);
  xmlReader.setErrorHandler(handler);

  bool ok = xmlReader.parse(source);

  if (ok) {
    void *engineContext = NULL;

    CEngineResponseMessage *resp = new CEngineResponseMessage;
    CBatchEngineController *controller = new CBatchEngineController;

    // copy the properties data ...
    mediate_convolution_t properties = *(handler->properties()); // blot copy

    if (!cmd->outputDir.isEmpty() && cmd->outputDir.size() < FILENAME_BUFFER_LENGTH-1) {
      // override the output directory
      strcpy(properties.general.outputFile, cmd->outputDir.toAscii().data());
    }

    if (mediateXsconvCreateContext(&engineContext, resp) != 0) {
      retCode = 1;
    }
    else {

      const QList<QString> &filenames = cmd->filenames;

      if (!filenames.isEmpty()) {

	// loop over files ...
	QList<QString>::const_iterator it = filenames.begin();
	while (!retCode && it != filenames.end()) {

	  if (!it->isEmpty() && it->size() < FILENAME_BUFFER_LENGTH-1) {
	    strcpy(properties.general.inputFile, it->toAscii().data());

     mediateRequestConvolution(engineContext, &properties, resp);
	    retCode = mediateConvolutionCalculate(engineContext,resp);
	    resp->process(controller);
	  }

	  ++it;
	}

      }
      else {
	// use the current input file
	mediateRequestConvolution(engineContext, &properties, resp);
	retCode = mediateConvolutionCalculate(engineContext,resp);
	resp->process(controller);
      }

      if (mediateXsconvDestroyContext(engineContext, resp) != 0) {
	retCode = 1;
      }
    }

    delete resp;
    delete controller;
  }
  else {
    retCode = 1;
  }

  delete handler;
  delete source;
  delete file;

  return retCode;
}

int batchProcessRing(commands_t *cmd)
{
  TRACE("batchProcessRing");

  return 0;
}

int batchProcessUsamp(commands_t *cmd)
{
  TRACE("batchProcessUsamp");

  return 0;
}


int analyseProjectQdoasDirectory(void *engineContext, CBatchEngineController *controller,
				 const QString &dir, const QString &filter, bool recursive)
{
  TRACE("analyseProjectQdoasDirectory " << dir.toStdString());

  int retCode = 0;
  QFileInfoList entries;
  QFileInfoList::iterator it;

  QDir directory(dir);

  // first consder sub directories ...
  if (recursive) {
    entries = directory.entryInfoList(); // all entries ... but only take directories on this pass

    it = entries.begin();
    while (!retCode && it != entries.end()) {
      if (it->isDir() && !it->fileName().startsWith('.')) {

        retCode = analyseProjectQdoasDirectory(engineContext, controller, it->filePath(), filter, true);
      }
      ++it;
    }
  }

  // now the files that match the filters
  if (filter.isEmpty())
    entries = directory.entryInfoList();
  else
    entries = directory.entryInfoList(QStringList(filter));

  it = entries.begin();
  while (!retCode && it != entries.end()) {
    if (it->isFile()) {

      retCode = analyseProjectQdoasFile(engineContext, controller, it->filePath());
    }
    ++it;
  }

  return retCode;
}
