/*------------------------------------------------------------------------
 *   Generates elastic model properties (vp,vs,density, Cij coefficients) on the fly
 *
 *   depending on model dimension in vertical direction and local variable "h"
 *   this function can generate a
 *   	-> homogeneneous full space
 *   	-> layer over half space
 * 		-> spherical perturbation in the middle of the model 
 * 
 *  ----------------------------------------------------------------------*/

#include "fd.h"

void model_elastic(float ***rho, float ***pi, float ***u,
		float ***C11, float ***C12, float ***C13,
		float ***C22, float ***C23, float ***C33,
		float ***C44, float ***C55, float ***C66)
{
    /*--------------------------------------------------------------------------*/
    /* extern variables */
    extern float DY;
    extern int NX, NY, NZ, NXG, NYG, NZG, POS[4], L, MYID;
    extern char MFILE[STRING_SIZE];
    extern int WRITE_MODELFILES;
    extern int READMOD;
    extern FILE *FP;

    /* local variables */
    float muv, piv;
    float Vpv, Vsv, Rho, Poi;
    float 	C_11, C_22, C_33,
    		C_44, C_55, C_66,
        	C_12, C_13, C_23;
	float eps_1, eps_2, delta_1, delta_2, delta_3, gamma_1, gamma_2;
    float ***vpv = NULL, ***vsv = NULL, ***epsx = NULL, ***epsy = NULL, ***gamx = NULL;
    float ***delx = NULL, ***dely = NULL, ***delxy = NULL, ***gamy = NULL;
    float y;
    int i, j, k, ii, jj, kk;
    char modfile[STRING_SIZE];

    // Model parameters for model generation.
    extern float VPV1, VSV1, EPSX1, EPSY1, DELX1, DELY1, DELXY1;
    extern float GAMX1, GAMY1, RHO1, DH1;
    extern float VPV2, VSV2, EPSX2, EPSY2, DELX2, DELY2, DELXY2;
    extern float GAMX2, GAMY2, RHO2, DH2;

    /*-----------------material property definition -------------------------*/
    /* x=1, y=2 in Tsvankin [1997] (e.g.) epsx=epsion1 & epsy=epsilon2 */

    /* Parameters for layer 1 */
    float vpv1 = 3000.0,
          poi1 = 0.25,
          vsv1 = vpv1 * sqrt((1 - 2 * poi1) / (2 - 2 * poi1)),
          // vsv1 = 1732.0508075688772,
          epsx1 = 0.0,
          epsy1 = 0.0,
          delx1 = -0.0,
          dely1 = 0.0,
          delxy1 = 0,
          gamx1 = 0.0,
          gamy1 = 0.0,
          rho1 = 2000.0,
          h = 960;

    /* Parameters for layer 2 */
    float vpv2 = 3000.0,
          poi2 = 0.25,  // poi2 = 0.5*(vp2vs2 - 2) / (vp2vs2 -1),
          vsv2 = vpv2 * sqrt((1 - 2 * poi2) / (2 - 2 * poi2)),
          // vsv1 = 1732.0508075688772,
          epsx2 = 0,
          epsy2 = 0,
          delx2 = 0,
          dely2 = 0,
          delxy2 = 0,
          gamx2 = 0.2,
          gamy2 = 0.2,
          rho2 = 2000.0,
          dh = 200;

    if (READMOD == -1) {
        float tmp;
        vpv1 = VPV1;
        vsv1 = VSV1;
        tmp = (vpv1 * vpv1) / (vsv1 * vsv1);
        poi1 = 0.5*(tmp - 2) / (tmp -1);
        epsx1 = EPSX1;
        epsy1 = EPSY1;
        delx1 = DELX1;
        dely1 = DELY1;
        delxy1 = DELXY1;
        gamx1 = GAMX1;
        gamy1 = GAMY1;
        rho1 = RHO1;
        h = DH1;

        vpv2 = VPV2;
        vsv2 = VSV2;
        tmp = (vpv2 * vpv2) / (vsv2 * vsv2);
        poi2 = 0.5*(tmp - 2) / (tmp -1);
        epsx2 = EPSX2;
        epsy2 = EPSY2;
        delx2 = DELX2;
        dely2 = DELY2;
        delxy2 = DELXY2;
        gamx2 = GAMX2;
        gamy2 = GAMY2;
        rho2 = RHO2;
        dh = DH2;
    }

    // parameters for a perturbation
    const float pertRad = 5.0,
                relPertVpv = 0.0;

    if (WRITE_MODELFILES == 1)
    {
        vpv = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
        vsv = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
        epsx = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
        epsy = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
        delx = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
        dely = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
        delxy = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
        gamx = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
        gamy = f3tensor(0, NY + 1, 0, NX + 1, 0, NZ + 1);
    }

    /* Elastic simulation. */
    if (L == 0)
    {
        /* loop over global grid */
        fprintf(FP, "In HH elastic MYID=%d, POS[1]=%d, POS[2]=%d,POS[3]=%d \n\n", MYID, POS[1], POS[2], POS[3]);
        for (k = 1; k <= NZG; k++)
        {
            for (i = 1; i <= NXG; i++)
            {
                for (j = 1; j <= NYG; j++)
                {
                    /*note that "y" is used for the vertical coordinate*/
                    /* calculate vertical coordinate in m */

                    y = (float)j * DY;
                    /* two layer case */

                    Vpv = vpv1;
                    Poi = poi1;
                    eps_1 = epsx1;
                    eps_2 = epsy1;
                    delta_1 = delx1;
                    delta_2 = dely1;
                    delta_3 = delxy1;
                    gamma_1 = gamx1;
                    gamma_2 = gamy1;
                    Rho = rho1;

                    if ((y >= h) && (y <= h + dh))
                    {
                        Vpv = vpv2;
                        Poi = poi2;
                        eps_1 = epsx2;
                        eps_2 = epsy2;
                        delta_1 = delx2;
                        delta_2 = dely2;
                        delta_3 = delxy2;
                        gamma_1 = gamx2;
                        gamma_2 = gamy2;
                        Rho = rho2;
                    }

                    // Perturbation in the middle of the model.
                    if (((i - (NZG / 2)) * (i - (NZG / 2)) + (j - (NZG / 2)) * (j - (NZG / 2)) + (k - (NZG / 2)) * (k - (NZG / 2))) <= pertRad * pertRad)
                    {
                        Vpv += Vpv * relPertVpv;
                    }

                    Vsv = Vpv * sqrt((1 - 2 * Poi) / (2 - 2 * Poi));
                    muv = Vsv * Vsv * Rho;
                    piv = Vpv * Vpv * Rho;

                    /* only the PE which belongs to the current global gridpoint
					 * is saving model parameters in his local arrays */

                    if ((POS[1] == ((i - 1) / NX)) &&
                        (POS[2] == ((j - 1) / NY)) &&
                        (POS[3] == ((k - 1) / NZ)))
                    {
                        ii = i - POS[1] * NX;
                        jj = j - POS[2] * NY;
                        kk = k - POS[3] * NZ;

                        // leftovers from isotropic case -- necessary for PML
                        u[jj][ii][kk] = muv;
                        pi[jj][ii][kk] = piv;
                        /*VTI
						  C11[jj][ii][kk] = (1+2*Epsx)*Rho*Vpv*Vpv;
						  C22[jj][ii][kk] = C11[jj][ii][kk];
						  C33[jj][ii][kk] = Rho*Vpv*Vpv;
						  C66[jj][ii][kk] = Rho*Vsv*Vsv;
						  C12[jj][ii][kk] = C11[jj][ii][kk] - 2*C66[jj][ii][kk];
						  C13[jj][ii][kk]=Rho*sqrt((Vpv*Vpv-Vsv*Vsv)*((1+2*Delx)*Vpv*Vpv-Vsv*Vsv))-Rho*Vsv*Vsv;
						  C23[jj][ii][kk]=C13[jj][ii][kk];
						  C44[jj][ii][kk]=Rho*Vsv*Vsv/(1+2*Gamx);
						  C55[jj][ii][kk]=Rho*Vsv*Vsv/(1+2*Gamx);
						  rho[jj][ii][kk]=Rho;                        
						 */

                        // Humane notation - third axis is vertical
                        C_33 = Rho * Vpv * Vpv;
                        C_55 = Rho * Vsv * Vsv;
                        C_66 = (1 + 2 * gamma_1) * C_55;
                        C_11 = (1 + 2 * eps_2) * C_33;
                        C_44 = C_66 / (1 + 2 * gamma_2);
						C_22 = (1 + 2 * eps_1) * C_33;
                        C_13 = -C_55 + sqrt(2 * delta_2 * C_33 * (C_33 - C_55) + (C_33 - C_55) * (C_33 - C_55));
						C_12 = -C_66 + sqrt(2 * delta_3 * C_11 * (C_11 - C_66) + (C_11 - C_66) * (C_11 - C_66));
						C_23 = -C_44 + sqrt(2 * delta_1 * C_33 * (C_33 - C_44) + (C_33 - C_44) * (C_33 - C_44));
						
                        // We need to convert here from humane notation
                        // to ASOFI3D notation, where 2nd axis is vertical
                        // instead of the 3rd axis.
						// Conversion is done in the following way:
                        // C33 <-> C22
                        // C55 <-> C66
                        // C12 <-> C13

                        // ORTHO PARAMETERS FROM SAVA
                        C_11 = 10.8e9;
                        C_12 =  2.2e9;
                        C_13 =  1.9e9;
                        C_22 = 11.3e9;
                        C_23 =  1.7e9;
                        C_33 =  8.5e9;
                        C_44 =  3.6e9;
                        C_55 =  3.9e9;
                        C_66 =  4.3e9;
                        Rho = 2000;

                        // ISO PARAMETERS FROM SAVA
                        // C_11 = 8.0e9;
                        // C_12 = 2.67e9;
                        // C_13 = 2.67e9;
                        // C_22 = 8.0e9;
                        // C_23 = 2.67e9;
                        // C_33 = 8.0e9;
                        // C_44 = 2.67e9;
                        // C_55 = 2.67e9;
                        // C_66 = 2.67e9;
                        // Rho = 2000;


						C11[jj][ii][kk] = C_11;
                        C33[jj][ii][kk] = C_22;
                        C22[jj][ii][kk] = C_33;

                        C44[jj][ii][kk] = C_44;
                        C66[jj][ii][kk] = C_55;
                        C55[jj][ii][kk] = C_66;

                        C13[jj][ii][kk] = C_12;
                        C12[jj][ii][kk] = C_13;
                        C23[jj][ii][kk] = C_23;

                        rho[jj][ii][kk] = Rho;

                        if (WRITE_MODELFILES == 1)
                        {
                            vpv[jj][ii][kk] = Vpv;
                            vsv[jj][ii][kk] = Vsv;
                        }
                    }
                }
            }
        }
    }

    /* each PE writes his model to disk */

    /* all models are written to file we need to add anisotropic models output here*/
    if (WRITE_MODELFILES == 1)
    {
        sprintf(modfile, "%s.SOFI3D.pi", MFILE);
        writemod(modfile, pi, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0)
            mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.u", MFILE);
        writemod(modfile, u, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0)
            mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.vp", MFILE);
        writemod(modfile, vpv, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0)
            mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.vs", MFILE);
        writemod(modfile, vsv, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0)
            mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.rho", MFILE);
        writemod(modfile, rho, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0)
            mergemod(modfile, 3);

        // Notice that the stiffness parameters are written
        // to disk in the conventional notation (third axis is vertical);
        // that's why there is a mismatch between filenames and variable names.
        sprintf(modfile, "%s.SOFI3D.C11", MFILE);
        writemod(modfile, C11, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.C22", MFILE);
        writemod(modfile, C33, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.C33", MFILE);
        writemod(modfile, C22, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.C44", MFILE);
        writemod(modfile, C44, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.C55", MFILE);
        writemod(modfile, C66, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.C66", MFILE);
        writemod(modfile, C55, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.C12", MFILE);
        writemod(modfile, C13, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.C13", MFILE);
        writemod(modfile, C12, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);

        sprintf(modfile, "%s.SOFI3D.C23", MFILE);
        writemod(modfile, C23, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0) mergemod(modfile, 3);
    }

    /* only density is written to file */
    if (WRITE_MODELFILES == 2)
    {
        sprintf(modfile, "%s.SOFI3D.rho", MFILE);
        writemod(modfile, rho, 3);
        MPI_Barrier(MPI_COMM_WORLD);
        if (MYID == 0)
            mergemod(modfile, 3);
    }

    if (WRITE_MODELFILES == 1)
    {
        free_f3tensor(vpv, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
        free_f3tensor(vsv, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
        free_f3tensor(epsx, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
        free_f3tensor(epsy, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
        free_f3tensor(delx, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
        free_f3tensor(dely, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
        free_f3tensor(delxy, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
        free_f3tensor(gamx, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
        free_f3tensor(gamy, 0, NY + 1, 0, NX + 1, 0, NZ + 1);
    }
}
