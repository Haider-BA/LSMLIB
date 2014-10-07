/*
 * File:        COMPUTE_NORMAL_VELOCITY_TERM_FOR_LSE_RHS_3D.c
 * Copyrights:  (c) 2005 The Trustees of Princeton University and Board of
 *                  Regents of the University of Texas.  All rights reserved.
 *              (c) 2009 Kevin T. Chu.  All rights reserved.
 * Revision:    $Revision$
 * Modified:    $Date$
 * Description: MATLAB MEX-file for adding the contribution from a
 *              normal velocity term to the RHS of level set equation
 *            
 */

/*=======================================================================
 *
 * COMPUTE_NORMAL_VELOCITY_TERM_FOR_LSE_RHS_3D() computes the 
 * contribution of a normal velocity term to the right-hand side of 
 * the level set equation.
 *
 * Usage:  lse_rhs = COMPUTE_NORMAL_VELOCITY_TERM_FOR_LSE_RHS_3D( ...
 *                     phi, ghostcell_width, ...
 *                     normal_velocity, ...
 *                     phi_x_plus, phi_y_plus, phi_z_plus, ...
 *                     phi_x_minus, phi_y_minus, phi_z_minus)
 *
 * Arguments:
 * - phi:               level set function
 * - ghostcell_width:   ghostcell width for phi
 * - normal_velocity:   normal velocity
 * - phi_x_plus:        x-component of plus HJ ENO derivative
 * - phi_y_plus:        y-component of plus HJ ENO derivative
 * - phi_z_plus:        z-component of plus HJ ENO derivative
 * - phi_x_minus:       x-component of minus HJ ENO derivative
 * - phi_y_minus:       y-component of minus HJ ENO derivative
 * - phi_z_minus:       z-component of minus HJ ENO derivative
 *
 * Return value:
 * - lse_rhs:           normal velocity contribution to right-hand side
 *                      level set evolution equation
 *
 * NOTES:
 * - The phi_x_plus, phi_y_plus, phi_z_plus, phi_x_minus, phi_y_minus, 
 *   and phi_z_minus arrays are assumed to be the same size.
 *
 * - All data arrays are assumed to be in the order generated by the 
 *   MATLAB meshgrid() function.  That is, data corresponding to the 
 *   point (x_i,y_j,z_k) is stored at index (j,i,k).  The spatial 
 *   derivative functions provided by LSMLIB return data arrays ordered 
 *   in this way.
 *
 * - The returned lse_rhs array is the same size as phi.  However, only
 *   the values of the RHS of the level set evolution equation within 
 *   the _interior_ of the computational grid are computed.  In other 
 *   words, values of the RHS in the ghostcells are _not_ computed; the
 *   value in the ghostcells is set to 0.
 *
 *=======================================================================*/

#include "mex.h"
#include "matrix.h"
#include "LSMLIB_config.h"
#include "lsm_level_set_evolution3d.h"

/* Input Arguments */ 
#define PHI             (prhs[0])
#define GHOSTCELL_WIDTH (prhs[1])
#define NORMAL_VELOCITY (prhs[2])
#define PHI_X_PLUS      (prhs[3])
#define PHI_Y_PLUS      (prhs[4])
#define PHI_Z_PLUS      (prhs[5])
#define PHI_X_MINUS     (prhs[6])
#define PHI_Y_MINUS     (prhs[7])
#define PHI_Z_MINUS     (prhs[8])

/* Output Arguments */ 
#define LSE_RHS         (plhs[0])

/* Macros */ 
#define NDIM            (3)


void mexFunction( int nlhs, mxArray *plhs[], 
		  int nrhs, const mxArray*prhs[] )
     
{ 

  /* data variables */
  int ilo_phi_gb, ihi_phi_gb, jlo_phi_gb, jhi_phi_gb, klo_phi_gb, khi_phi_gb;
  int phi_ghostcell_width;
  LSMLIB_REAL *normal_velocity; 
  int ilo_vel_gb, ihi_vel_gb, jlo_vel_gb, jhi_vel_gb, klo_vel_gb, khi_vel_gb;
  LSMLIB_REAL *phi_x_plus, *phi_y_plus, *phi_z_plus;
  LSMLIB_REAL *phi_x_minus, *phi_y_minus, *phi_z_minus;
  int ilo_grad_phi_gb, ihi_grad_phi_gb;
  int jlo_grad_phi_gb, jhi_grad_phi_gb;
  int klo_grad_phi_gb, khi_grad_phi_gb;
  LSMLIB_REAL *lse_rhs; 
  int ilo_lse_rhs_gb, ihi_lse_rhs_gb;
  int jlo_lse_rhs_gb, jhi_lse_rhs_gb;
  int klo_lse_rhs_gb, khi_lse_rhs_gb;
  int ilo_fb, ihi_fb, jlo_fb, jhi_fb, klo_fb, khi_fb;

  /* array dimension information */
  int num_data_array_dims;
  const int *data_array_dims_in;
  int data_array_dims_out[NDIM];

  /* auxilliary variables */
  int num_grid_cells;
  int i;  
  
  /* Check for proper number of arguments */
  
  if (nrhs != 9) { 
    mexErrMsgTxt("Nine input arguments required."); 
  } else if (nlhs > 1) {
    mexErrMsgTxt("Too many output arguments."); 
  } 
    
  /* Check that the inputs have the correct floating-point precision */
#ifdef LSMLIB_DOUBLE_PRECISION 
    if (!mxIsDouble(PHI)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for double-precision but phi is single-precision");
    }
    if (!mxIsDouble(NORMAL_VELOCITY)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for double-precision but normal_velocity is single-precision");
    }
    if (!mxIsDouble(PHI_X_PLUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for double-precision but phi_x_plus is single-precision");
    }
    if (!mxIsDouble(PHI_Y_PLUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for double-precision but phi_y_plus is single-precision");
    }
    if (!mxIsDouble(PHI_Z_PLUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for double-precision but phi_z_plus is single-precision");
    }
    if (!mxIsDouble(PHI_X_MINUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for double-precision but phi_x_minus is single-precision");
    }
    if (!mxIsDouble(PHI_Y_MINUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for double-precision but phi_y_minus is single-precision");
    }
    if (!mxIsDouble(PHI_Z_MINUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for double-precision but phi_z_minus is single-precision");
    }
#else       
    if (!mxIsSingle(PHI)) {           
      mexErrMsgTxt("Incompatible precision: LSMLIB built for single-precision but phi is double-precision");
    }
    if (!mxIsSingle(NORMAL_VELOCITY)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for single-precision but normal_velocity is double-precision");
    }
    if (!mxIsSingle(PHI_X_PLUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for single-precision but phi_x_plus is double-precision");
    }
    if (!mxIsSingle(PHI_Y_PLUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for single-precision but phi_y_plus is double-precision");
    }
    if (!mxIsSingle(PHI_Z_PLUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for single-precision but phi_z_plus is double-precision");
    }
    if (!mxIsSingle(PHI_X_MINUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for single-precision but phi_x_minus is double-precision");
    }
    if (!mxIsSingle(PHI_Y_MINUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for single-precision but phi_y_minus is double-precision");
    }
    if (!mxIsSingle(PHI_Z_MINUS)) {
      mexErrMsgTxt("Incompatible precision: LSMLIB built for single-precision but phi_z_minus is double-precision");
    }
#endif

  /* Parameter Checks */
  num_data_array_dims = mxGetNumberOfDimensions(PHI);
  if (num_data_array_dims != 3) {
    mexErrMsgTxt("phi should be a 3 dimensional array."); 
  }

  /* Assign pointers for normal velocity and grad(phi) */
  normal_velocity = (LSMLIB_REAL*) mxGetPr(NORMAL_VELOCITY);
  phi_x_plus      = (LSMLIB_REAL*) mxGetPr(PHI_X_PLUS);
  phi_y_plus      = (LSMLIB_REAL*) mxGetPr(PHI_Y_PLUS);
  phi_z_plus      = (LSMLIB_REAL*) mxGetPr(PHI_Z_PLUS);
  phi_x_minus     = (LSMLIB_REAL*) mxGetPr(PHI_X_MINUS);
  phi_y_minus     = (LSMLIB_REAL*) mxGetPr(PHI_Y_MINUS);
  phi_z_minus     = (LSMLIB_REAL*) mxGetPr(PHI_Z_MINUS);
      
  /* Get size of data */
  data_array_dims_in = mxGetDimensions(PHI);
  ilo_phi_gb = 1;
  ihi_phi_gb = data_array_dims_in[0];
  jlo_phi_gb = 1;
  jhi_phi_gb = data_array_dims_in[1];
  klo_phi_gb = 1;
  khi_phi_gb = data_array_dims_in[2];

  data_array_dims_in = mxGetDimensions(NORMAL_VELOCITY);
  ilo_vel_gb = 1;
  ihi_vel_gb = data_array_dims_in[0];
  jlo_vel_gb = 1;
  jhi_vel_gb = data_array_dims_in[1];
  klo_vel_gb = 1;
  khi_vel_gb = data_array_dims_in[2];

  /* if necessary, shift ghostbox for velocity to be */ 
  /* centered with respect to the ghostbox for phi.  */
  if (ihi_vel_gb != ihi_phi_gb) {
    int shift = (ihi_phi_gb-ihi_vel_gb)/2;
    ilo_vel_gb += shift;
    ihi_vel_gb += shift;
  }
  if (jhi_vel_gb != jhi_phi_gb) {
    int shift = (jhi_phi_gb-jhi_vel_gb)/2;
    jlo_vel_gb += shift;
    jhi_vel_gb += shift;
  }
  if (khi_vel_gb != khi_phi_gb) {
    int shift = (khi_phi_gb-khi_vel_gb)/2;
    klo_vel_gb += shift;
    khi_vel_gb += shift;
  }

  data_array_dims_in = mxGetDimensions(PHI_X_PLUS);
  ilo_grad_phi_gb = 1;
  ihi_grad_phi_gb = data_array_dims_in[0];
  jlo_grad_phi_gb = 1;
  jhi_grad_phi_gb = data_array_dims_in[1];
  klo_grad_phi_gb = 1;
  khi_grad_phi_gb = data_array_dims_in[2];

  /* if necessary, shift ghostbox for grad(phi) to be */ 
  /* centered with respect to the ghostbox for phi.  */
  if (ihi_grad_phi_gb != ihi_phi_gb) {
    int shift = (ihi_phi_gb-ihi_grad_phi_gb)/2;
    ilo_grad_phi_gb += shift;
    ihi_grad_phi_gb += shift;
  }
  if (jhi_grad_phi_gb != jhi_phi_gb) {
    int shift = (jhi_phi_gb-jhi_grad_phi_gb)/2;
    jlo_grad_phi_gb += shift;
    jhi_grad_phi_gb += shift;
  }
  if (khi_grad_phi_gb != khi_phi_gb) {
    int shift = (khi_phi_gb-khi_grad_phi_gb)/2;
    klo_grad_phi_gb += shift;
    khi_grad_phi_gb += shift;
  }

  /* Create matrix for normal velocity contribution to RHS */
  /* of level set equation                                 */
  ilo_lse_rhs_gb = ilo_phi_gb;
  ihi_lse_rhs_gb = ihi_phi_gb;
  jlo_lse_rhs_gb = jlo_phi_gb;
  jhi_lse_rhs_gb = jhi_phi_gb;
  klo_lse_rhs_gb = klo_phi_gb;
  khi_lse_rhs_gb = khi_phi_gb;
  data_array_dims_out[0] = ihi_lse_rhs_gb-ilo_lse_rhs_gb+1;
  data_array_dims_out[1] = jhi_lse_rhs_gb-jlo_lse_rhs_gb+1;
  data_array_dims_out[2] = khi_lse_rhs_gb-klo_lse_rhs_gb+1;
#ifdef LSMLIB_DOUBLE_PRECISION
  LSE_RHS = mxCreateNumericArray(NDIM, data_array_dims_out,
                                 mxDOUBLE_CLASS, mxREAL);
#else
  LSE_RHS = mxCreateNumericArray(NDIM, data_array_dims_out,
                                 mxSINGLE_CLASS, mxREAL);
#endif
  lse_rhs = (LSMLIB_REAL*) mxGetPr(LSE_RHS); 

  /* zero out LSE_RHS */
  num_grid_cells = data_array_dims_out[0]*data_array_dims_out[1]
                 *  data_array_dims_out[2];
  for (i = 0; i < num_grid_cells; i++, lse_rhs++) {
    (*lse_rhs) = 0.0;
  }
  /* reset lse_rhs to start of data array */
  lse_rhs = (LSMLIB_REAL*) mxGetPr(LSE_RHS);  


  /* Do the actual computations in a Fortran 77 subroutine */
  phi_ghostcell_width = mxGetPr(GHOSTCELL_WIDTH)[0];
  ilo_fb = ilo_lse_rhs_gb + phi_ghostcell_width;
  ihi_fb = ihi_lse_rhs_gb - phi_ghostcell_width;
  jlo_fb = jlo_lse_rhs_gb + phi_ghostcell_width;
  jhi_fb = jhi_lse_rhs_gb - phi_ghostcell_width;
  klo_fb = klo_lse_rhs_gb + phi_ghostcell_width;
  khi_fb = khi_lse_rhs_gb - phi_ghostcell_width;
  LSM3D_ADD_NORMAL_VEL_TERM_TO_LSE_RHS(
    lse_rhs,
    &ilo_lse_rhs_gb, &ihi_lse_rhs_gb,
    &jlo_lse_rhs_gb, &jhi_lse_rhs_gb,
    &klo_lse_rhs_gb, &khi_lse_rhs_gb,
    phi_x_plus, phi_y_plus, phi_z_plus,
    &ilo_grad_phi_gb, &ihi_grad_phi_gb,
    &jlo_grad_phi_gb, &jhi_grad_phi_gb,
    &klo_grad_phi_gb, &khi_grad_phi_gb,
    phi_x_minus, phi_y_minus, phi_z_minus,
    &ilo_grad_phi_gb, &ihi_grad_phi_gb,
    &jlo_grad_phi_gb, &jhi_grad_phi_gb,
    &klo_grad_phi_gb, &khi_grad_phi_gb,
    normal_velocity, 
    &ilo_vel_gb, &ihi_vel_gb, 
    &jlo_vel_gb, &jhi_vel_gb, 
    &klo_vel_gb, &khi_vel_gb, 
    &ilo_fb, &ihi_fb, 
    &jlo_fb, &jhi_fb,
    &klo_fb, &khi_fb);

  return;
}
