
#include "CProjectConfigItem.h"


CAnalysisWindowConfigItem::CAnalysisWindowConfigItem()
{
  initializeMediateAnalysisWindow(&m_awProp);
}

CAnalysisWindowConfigItem::~CAnalysisWindowConfigItem()
{
}

bool CAnalysisWindowConfigItem::setName(const QString &name)
{
  if (name.length() < (int)sizeof(m_awProp.name)) {

    strcpy(m_awProp.name, name.toAscii().data());
    m_name = name;

    return true;
  }
  
  return false;
}

const QString& CAnalysisWindowConfigItem::name(void) const
{
  return m_name;
}

mediate_analysis_window_t* CAnalysisWindowConfigItem::properties(void)
{
  // WARNING : allows (by design) poking at the internals ...
  return &m_awProp;
}

const mediate_analysis_window_t* CAnalysisWindowConfigItem::properties(void) const
{
  return &m_awProp;
}

//------------------------------------------------------------

CProjectConfigItem::CProjectConfigItem()
{
  m_root = new CProjectConfigFolder("root", true);

  initializeMediateProject(&m_projProp);
}

CProjectConfigItem::~CProjectConfigItem()
{
  while (!m_awItemList.empty())
    delete m_awItemList.takeFirst();

  delete m_root;
}

void CProjectConfigItem::setName(const QString &name)
{
  m_name = name;
}

const QString& CProjectConfigItem::name(void) const
{
  return m_name;
}

mediate_project_t* CProjectConfigItem::properties(void)
{
  // WARNING : allows (by design) poking at the internals ...

  return &m_projProp;
}

const mediate_project_t* CProjectConfigItem::properties(void) const
{
  return &m_projProp;
}

CAnalysisWindowConfigItem* CProjectConfigItem::issueNewAnalysisWindowItem(void)
{
  CAnalysisWindowConfigItem *tmp = new CAnalysisWindowConfigItem;
  m_awItemList.push_back(tmp);
  
  // retains ownership
  return tmp;
}

const QList<const CAnalysisWindowConfigItem*>& CProjectConfigItem::analysisWindowItems(void) const
{
  return m_awItemList;
}

CProjectConfigTreeNode* CProjectConfigItem::rootNode(void)
{
  return m_root;
}

const CProjectConfigTreeNode* CProjectConfigItem::rootNode(void) const
{
  return m_root;
}


//------------------------------------------------------------

CSiteConfigItem::CSiteConfigItem() :
  m_longitude(0.0),
  m_latitude(0.0),
  m_altitude(0.0)
{
}

CSiteConfigItem::~CSiteConfigItem()
{
}

//------------------------------------------------------------

CSymbolConfigItem::CSymbolConfigItem(const QString &name, const QString &description) :
  m_name(name),
  m_description(description)
{
}

CSymbolConfigItem::~CSymbolConfigItem()
{
}

