#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#define CALLDEF(name, n)  {#name, (DL_FUNC) &name, n}

extern "C" {

  SEXP boom_rinterface_fit_finite_mixture_(
      SEXP rmixture_component_list,
      SEXP rmixing_weight_prior,
      SEXP rniter,
      SEXP rping,
      SEXP rknown_source,
      SEXP rseed);

  SEXP boom_rinterface_fit_conditional_mixture_(
      SEXP rmixture_component_list,
      SEXP rmixing_distribution_prior,
      SEXP rmixture_design_matrix,
      SEXP rniter,
      SEXP rping,
      SEXP rknown_source,
      SEXP rseed);

  SEXP composite_hmm_wrapper_(SEXP rmixture_components,
                              SEXP rmarkov_model_prior,
                              SEXP rniter,
                              SEXP rping,
                              SEXP rseed);

  SEXP nested_hmm_wrapper_(SEXP r_streams,
                           SEXP r_eos_label,
                           SEXP r_nested_hmm_prior,
                           SEXP r_niter,
                           SEXP r_burn,
                           SEXP r_ping,
                           SEXP r_threads,
                           SEXP r_seed,
                           SEXP r_print_suf_level);
  
  static R_CallMethodDef boom_mix_arg_description[] = {
    CALLDEF(boom_rinterface_fit_finite_mixture_, 6),
    CALLDEF(boom_rinterface_fit_conditional_mixture_, 7),
    CALLDEF(composite_hmm_wrapper_, 5),
    CALLDEF(nested_hmm_wrapper_, 9),
    {NULL, NULL, 0}
  };

  void R_init_BoomMix(DllInfo *info) {
    R_registerRoutines(info,
                       NULL,
                       boom_mix_arg_description,
                       NULL,
                       NULL);
    R_useDynamicSymbols(info, FALSE);
  }

}  // extern "C"
