

#include "engine.h"
#include "doas.h"

typedef struct _engine
 {
  SPEC_INFO specInfo;                                                           // information on the current record
  FILE *specFp,*namesFp,*darkFp;                                                // pointers to the current files
  int nRecords;                                                                 // the total number of records
  int record;                                                                   // index of the current record
  int NDET;                                                                     // size of the detector
 }
ENGINE_CONTEXT;

ENGINE_CONTEXT engineContext;

int EngineSetFile(const char *fileName)
 {
 	int i;
 	SPEC_INFO *pSpecInfo;

  memset(&engineContext,0,sizeof(ENGINE_CONTEXT));
  NDET=engineContext.NDET=1024;
  pSpecInfo=&engineContext.specInfo;

  strcpy(pSpecInfo->fileName,fileName); //"E:\\My Ground Based Activities\\GB_Stations\\OHP_2005\\Data\\2007\\");
  //strcat(pSpecInfo->fileName,fileName);
//  sprintf(pSpecInfo->fileName,"E:/My Ground Based Activities/GB_Stations/OHP_2005/Data/2007/%s",fileName);

  pSpecInfo->project.instrumental.azimuthFlag=1;

     // ThrdInitSpecInfo to initialize the engine context with project information + vectors allocation

  if (((pSpecInfo->lembda=MEMORY_AllocDVector("ThrdInitSpecInfo ","lembda",0,engineContext.NDET-1))==NULL) ||
      ((pSpecInfo->spectrum=MEMORY_AllocDVector("ThrdInitSpecInfo ","spectrum",0,engineContext.NDET-1))==NULL))

   ERROR_SetLast("EngineSetFile",ERROR_TYPE_WARNING,ERROR_ID_ALLOC,"in EngineSetFile)");

  else
   {
    engineContext.specFp=fopen(pSpecInfo->fileName,"rb");

    SetPDA_EGG_Logger(pSpecInfo,engineContext.specFp);
    for (i=0;i<engineContext.NDET;i++)
     pSpecInfo->lembda[i]=(double)i+1.;
   }

  return pSpecInfo->recordNumber;
 }

void EngineReadFile(int indexRecord,double *x,double *y)
 {
 	SPEC_INFO *pSpecInfo;

 	pSpecInfo=&engineContext.specInfo;

 	ReliPDA_EGG_Logger(pSpecInfo,indexRecord,0,0,engineContext.specFp);

 	{
 		FILE *fp;
 		int i;
 		fp=fopen("toto.dat","a+t");
 		for (i=0;i<1024;i++)
 		 fprintf(fp,"%g %g\n",pSpecInfo->lembda[i],pSpecInfo->spectrum[i]);
 		fclose(fp);
 	}


 	memcpy((double *)x,(double *)pSpecInfo->lembda,sizeof(double)*engineContext.NDET);
  memcpy((double *)y,(double *)pSpecInfo->spectrum,sizeof(double)*engineContext.NDET);
 }
