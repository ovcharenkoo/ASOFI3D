/*------------------------------------------------------------------------
 * Reading (distributed) source positions, timeshift, centre frequency  and amplitude from SOURCE_FILE.
 *
 *  ------------------------------------------------------------------------ */

#include "fd.h"
#include "globvar.h"


float **sources(FILE * fpsrc, int *nsrc, int * stype){

	/* declaration of extern variables */
	extern int MYID,SRC_MF,SOURCE_TYPE;
	extern FILE *FP;
	extern float DX, DY, DZ;
	extern float TS,REFSRC[3],SRCTSHIFT,FC,AMP;

	float **srcpos;
	int l;
	float xsrc, ysrc, zsrc, tshift, fc=0.0;
	char cline[STRING_SIZE];

	if (MYID==0){
		fprintf(FP,"\n **Message from function source (written by PE %d):\n",MYID);

		srcpos=fmatrix(1,6,1,*nsrc);
		/* stype=(int *)malloc(*nsrc*sizeof(int)); */

		for (l = 1; l <= *nsrc; l++) {
			if (fgets(cline, STRING_SIZE-1, fpsrc) == NULL) {
				err("[%s] Error reading line %d from source file '%s'",
					__func__, l);
			};
			switch(sscanf(cline,"%f%f%f%f%f%f%i",&xsrc, &ysrc, &zsrc, &tshift, &srcpos[5][l], &srcpos[6][l], &stype[l])){
			case 0: xsrc=0.0;
			case 1: zsrc=0.0;
			case 2: ysrc=0.0;
			case 3: tshift=0.0;
			case 4: srcpos[5][l]=FC;
			case 5: srcpos[6][l]=AMP;
			case 6: stype[l]=SOURCE_TYPE;
			}
			/*note that "y" is used for the vertical coordinate */
			if(SRC_MF==1) { /* feet */
				srcpos[1][l]=iround(xsrc/DX)*DX/0.3048-REFSRC[0];
				srcpos[2][l]=iround(ysrc/DY)*DY/0.3048-REFSRC[1];
				srcpos[3][l]=iround(zsrc/DZ)*DZ/0.3048-REFSRC[2];
			}
			else {
				srcpos[1][l]=iround(xsrc/DX)*DX-REFSRC[0];
				srcpos[2][l]=iround(ysrc/DY)*DY-REFSRC[1];
				srcpos[3][l]=iround(zsrc/DZ)*DZ-REFSRC[2];
			}
			srcpos[4][l]=tshift+SRCTSHIFT*(l-1);
		}
		fclose(fpsrc);

		/* Compute maximum frequency */
		for (l=1;l<=*nsrc;l++)
			if (srcpos[5][l]>fc) fc=srcpos[5][l];
		fprintf(FP," Maximum frequency defined in source file: %6.2f Hz\n",fc);
		TS=1.0/fc;

	}

	if (MYID!=0) srcpos=fmatrix(1,6,1,*nsrc);
	/*if (MYID!=0) stype=(int *)malloc(*nsrc*sizeof(int)); */

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(&srcpos[1][1],(*nsrc)*6,MPI_FLOAT,0,MPI_COMM_WORLD);
	MPI_Bcast(&stype[1],*nsrc,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&TS,1,MPI_FLOAT,0,MPI_COMM_WORLD);


	if (MYID==0){
		fprintf(FP," Number of global source positions found: %i\n",*nsrc);
		fprintf(FP," If source positions are not exactly placed at a grid point, they are shifted to the nearest grid point.\n");

		if (*nsrc>50) fprintf(FP," The following table is quite large (%i lines) and will, thus, be truncated to the first 50 entries! \n",*nsrc);
		/* outputs all sources per each subdomain / node*/
		printf("      x\t\ty\t\tz\t\ttshift\tfc\tamp\tstype\n");
		if (*nsrc>50) {
			for (l=1;l<=50;l++) fprintf(FP,"#%4.0d %6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%d\n", l,srcpos[1][l],srcpos[2][l],srcpos[3][l],srcpos[4][l],srcpos[5][l],srcpos[6][l],stype[l]);
		}
		else for (l=1;l<=*nsrc;l++) fprintf(FP,"#%4.0d %6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%d\n", l,srcpos[1][l],srcpos[2][l],srcpos[3][l],srcpos[4][l],srcpos[5][l],srcpos[6][l],stype[l]);

		fprintf(FP,"\n\n");
	}
	MPI_Barrier(MPI_COMM_WORLD);

	return srcpos;
}

float **pwsources(int *nsrc, int * stype){ /* plane wave excitation */

	/* declaration of extern variables */
	extern float PLANE_WAVE_DEPTH, TS, DX, DZ, PLANE_WAVE_ANGLE;
	extern int MYID, NXG, NZG, SRCREC, FW,SOURCE_TYPE;
	extern FILE *FP;	

	float **srcpos, x, y, z, tan_phi;
	int  k, l, isrc=0, ixend, iyend;

	if (MYID==0){

		if (SRCREC){ /* if SRCREC=1 -> read source positions from file */
			fprintf(FP,"\n Source file is ignored: Plane wave excitation, only. \n");
		} 
		fprintf(FP," Computing source nodes for plane wave excitation.\n");
		fprintf(FP," depth= %5.2f meter, incidence angle= %5.2f degrees.\n",PLANE_WAVE_DEPTH, PLANE_WAVE_ANGLE);

		tan_phi=tan(PLANE_WAVE_ANGLE*PI/180.0);
		fprintf(FP," Message from function sources (written by PE %d):\n",MYID);				

		ixend=NXG-FW;
		iyend=NZG-FW;

		srcpos=fmatrix(1,6,1,*nsrc);

		fprintf(FP," x-range for plane wave: %d to %d gridpoints. \n",FW,ixend);
		fprintf(FP," y-range for plane wave: %d to %d gridpoints. \n",FW,iyend);

		srcpos=fmatrix(1,6,1,*nsrc);
		isrc=0;

		for (k=FW;k<=iyend;k++) for (l=FW;l<=ixend;l++){
			x=(float)l*DX;
			y=PLANE_WAVE_DEPTH+(tan_phi*x);;
			z=(float)k*DZ;
			isrc++;
			srcpos[1][isrc]=x;
			srcpos[2][isrc]=y;
			srcpos[3][isrc]=z;
			srcpos[4][isrc]=0.0;
			srcpos[5][isrc]=1.0/TS;
			srcpos[6][isrc]=1.0;
			stype[isrc]=SOURCE_TYPE;
		}

		/*double check if number of receivers match, counted within loop*/
		fprintf(FP," Number of source positions: %i\n",isrc);
	}

	if (MYID!=0) srcpos=fmatrix(1,6,1,*nsrc);
	/*if (MYID!=0) stype=(int *)malloc(*nsrc*sizeof(int)); */

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Bcast(&srcpos[1][1],(*nsrc)*6,MPI_FLOAT,0,MPI_COMM_WORLD);
	MPI_Bcast(&stype[1],*nsrc,MPI_INT,0,MPI_COMM_WORLD);

	if (MYID==0){
		fprintf(FP,"\n **Message from function source (written by PE %d):\n",MYID);
		fprintf(FP," Number of global source positions found: %i\n",*nsrc);
		if (*nsrc>50) fprintf(FP," The following table is quite large (%i lines) and will, thus, be truncated to the first 50 entries! \n",*nsrc);

		/* outputs all sources per each subdomain / node*/

		fprintf(FP,"      x\t\ty\t\tz\t\ttshift\tfc\tamp\tstype\n");
		if (*nsrc>50) {
			for (l=1;l<=50;l++) fprintf(FP,"#%4.0d %6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%d\n", l,srcpos[1][l],srcpos[2][l],srcpos[3][l],srcpos[4][l],srcpos[5][l],srcpos[6][l],stype[l]);
		}
		else for (l=1;l<=*nsrc;l++) fprintf(FP,"#%4.0d %6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%6.3f\t%d\n", l,srcpos[1][l],srcpos[2][l],srcpos[3][l],srcpos[4][l],srcpos[5][l],srcpos[6][l],stype[l]);


	}
	MPI_Barrier(MPI_COMM_WORLD);
	printf("\n\n");

	return srcpos;
}
