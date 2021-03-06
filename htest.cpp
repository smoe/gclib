#include "GBase.h"
#include "GArgs.h"
#include "GStr.h"
#include "GVec.hh"
#include "GHash.hh"
#include "GResUsage.h"
#include <cstdint>
#include <iostream>
#include <string>
//#include <tsl/hopscotch_map.h>
//#include <tsl/hopscotch_set.h>

#define USAGE "Usage:\n\
  htest textfile.. \n\
  \n\
 "
static void strFreeProc(pointer item) {
      GFREE(item);
}

struct HStrData {
	int cmd; // 0=add, 1=remove, 2=clear
	GStr str;
	HStrData(char* s=NULL, int c=0):cmd(c), str(s) { }
};

int loadStrings(FILE* f, GPVec<HStrData>& strgsuf, GPVec<HStrData>& strgs, int toLoad=0) {
  int num=0;
  GLineReader lr(f);
  char* line=NULL;
  while ((line=lr.nextLine())!=NULL) {
	  int len=strlen(line);
	  if (len<4) continue;
	  if (strcmp(line, "HCLR")==0) {
		  strgs.Add(new HStrData(NULL, 2));
		  strgsuf.Add(new HStrData(NULL, 2));
		  continue;
	  }
	  if (startsWith(line, "RM ")) {
	     strgsuf.Add(new HStrData(line+3,1) );
	     line[len-3]=0;
	     strgs.Add(new HStrData(line+3,1));
	     continue;
	  }
      strgsuf.Add(new HStrData(line));
      line[len-3]=0;
      strgs.Add(new HStrData(line));
	  num++;
	  if (toLoad && num>=toLoad) break;
  } //while line
  return num;
}

void showTimings(GResUsage swatch) {
 char *wtime=commaprintnum((uint64_t)swatch.elapsed());
 char *utime=commaprintnum((uint64_t)swatch.u_elapsed());
 char *stime=commaprintnum((uint64_t)swatch.s_elapsed());
 GMessage("Elapsed time (microseconds): %12s us\n", wtime);
 GMessage("                  user time: %12s us\n", utime);
 GMessage("                system time: %12s us\n", stime);
 GFREE(wtime);GFREE(utime);GFREE(stime);
}


void run_GHash(GResUsage& swatch, GPVec<HStrData> & hstrs, const char* label) {
	GHash<int> ghash;
	int num_add=0, num_rm=0, num_clr=0;
	GMessage("----------------- %s ----------------\n", label);
	ghash.Clear();
	swatch.start();
	for (int i=0;i<hstrs.Count();i++) {
			  switch (hstrs[i]->cmd) {
				case 0:ghash.fAdd(hstrs[i]->str.chars(), new int(1)); num_add++; break;
				case 1:ghash.Remove(hstrs[i]->str.chars()); num_rm++; break;
				case 2:ghash.Clear(); num_clr++; break;
			  }
	}
	ghash.Clear();
	swatch.stop();
	GMessage("  (%d inserts, %d deletions, %d clears)\n", num_add, num_rm, num_clr);
}
/*
void run_Hopscotch(GResUsage& swatch, GPVec<HStrData> & hstrs, const char* label) {
  int num_add=0, num_rm=0, num_clr=0;
  tsl::hopscotch_map<std::string, int> hsmap;
  GMessage("----------------- %s ----------------\n", label);
  swatch.start();
  for (int i=0;i<hstrs.Count();i++) {
	  switch (hstrs[i]->cmd) {
		case 0:hsmap.insert({hstrs[i]->str.chars(), 1}); num_add++; break;
		case 1:hsmap.erase(hstrs[i]->str.chars()); num_rm++; break;
		case 2:hsmap.clear(); num_clr++; break;
	  }
  }
  hsmap.clear();
  swatch.stop();
  GMessage("  (%d inserts, %d deletions, %d clears)\n", num_add, num_rm, num_clr);
}
*/
int main(int argc, char* argv[]) {
 GPVec<HStrData> strs;
 GPVec<HStrData> sufstrs;
 strs.setFreeItem(strFreeProc);
 sufstrs.setFreeItem(strFreeProc);
 //GArgs args(argc, argv, "hg:c:s:t:o:p:help;genomic-fasta=COV=PID=seq=out=disable-flag;test=");
 GArgs args(argc, argv, "h");
 //fprintf(stderr, "Command line was:\n");
 //args.printCmdLine(stderr);
 args.printError(USAGE, true);
 if (args.getOpt('h') || args.getOpt("help")) GMessage(USAGE);
 int numargs=args.startNonOpt();
 const char* a=NULL;
 FILE* f=NULL;
 int total=0;

 if (numargs==0) {
	 a="htest_data.lst";
	 f=fopen(a, "r");
	 if (f==NULL) GError("Error: could not open file %s !\n", a);
	 int num=loadStrings(f, sufstrs, strs, 600000);
	 total+=num;
	 GMessage("..loaded %d strings from file %s\n", total, a);
 }
 else {
	   while ((a=args.nextNonOpt())) {
		   f=fopen(a, "r");
		   if (f==NULL) GError("Error: could not open file %s !\n", a);
		   int num=loadStrings(f, sufstrs, strs, 600000);
		   total+=num;
	   }
  }
   GResUsage swatch;


   run_GHash(swatch, strs, "GHash no suffix");
   showTimings(swatch);

   run_GHash(swatch, sufstrs, "GHash w/ suffix");
   showTimings(swatch);
   /*
   run_Hopscotch(swatch, strs, "hopscotch no suffix");
   showTimings(swatch);

   run_Hopscotch(swatch, sufstrs, "hopscotch w/ suffix");
   showTimings(swatch);
   */
}
