#ifndef BOOM_STUDENT_MULTIVARIATE_STATE_SPACE_REGRESSION_HPP_
#define BOOM_STUDENT_MULTIVARIATE_STATE_SPACE_REGRESSION_HPP_
/*
  Copyright (C) 2022 Steven L. Scott

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License along
  with this library; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "Models/IndependentMvnModel.hpp"
#include "Models/Glm/Glm.hpp"
#include "Models/Glm/IndependentRegressionModels.hpp"
#include "Models/Glm/TRegression.hpp"
#include "Models/StateSpace/Multivariate/StateModels/SharedStateModel.hpp"
#include "Models/StateSpace/StateSpaceModel.hpp"
#include "Models/StateSpace/StateModelVector.hpp"
#include "Models/StateSpace/Multivariate/MultivariateStateSpaceModelBase.hpp"
#include "Models/StateSpace/Multivariate/MultivariateStateSpaceRegressionModel.hpp"

#include "cpputil/report_error.hpp"

namespace BoomStateSpaceTesting{
  class StudentMvssRegressionModelTest;
}

namespace BOOM {

  //===========================================================================
  // A scalar response value, paired with a set of predictor variables, at a
  // given point in time.
  class StudentMultivariateTimeSeriesRegressionData
      : public MultivariateTimeSeriesRegressionData {
   public:
    // Args:
    //   y: The response variable.
    //   x: A vector of predictors.
    //   series: The identifier of the time series (0.. number of series - 1) to
    //     which this observation belongs.
    //   timestamp: The time-index of the time series (0.. sample_size - 1)
    //     containing this observation.
    StudentMultivariateTimeSeriesRegressionData(double y,
                                                const Vector &x,
                                                int series,
                                                int timestamp);

    // As above, but y and x are Ptr's.  If the same X's apply to each time
    // series in Y, then this constructor is more space efficient than the one
    // above, because multiple Ptr's can point the the same predictor vector.
    StudentMultivariateTimeSeriesRegressionData(
        const Ptr<DoubleData> &y,
        const Ptr<VectorData> &x,
        int series,
        int timestamp);

    MultivariateTimeSeriesRegressionData *clone() const override {
      return new StudentMultivariateTimeSeriesRegressionData(*this);
    }

    // Override the weight() method in GlmData, which is a grandparent class.
    double weight() const override {return weight_;}
    void set_weight(double weight) {weight_ = weight;}

    std::ostream &display(std::ostream &out) const override {
      MultivariateTimeSeriesRegressionData::display(out)
          << "weight " << weight_ << "\n";
      return out;
    }

   private:
    double weight_;
  };

  //===========================================================================
  // A multivariate state space regression model describes a fixed dimensional
  // vector Y[t] as it moves throughout time.  The model is a state space model
  // of the form
  //
  //        Y[t] = Z[t] * alpha[t] + B * X[t] + epsilon[t]
  //  alpha[t+1] = T[t] * alpha[t] + R[t] * eta[t].
  //
  // The elements of epsilon[t] follow independent student T distributions where
  // element j has degrees of freedom nu[j] and scatter parameter (analogous to
  // a standard deviation, and equal to standard deviation as nu->infinity)
  // sigma[j].
  //
  // The student T distributions are defined in terms of a latent variable w[j,
  // t] with marginal distribuiton Gamma(nu[j], nu[j]).  This weight is to be
  // stored in data structure managing the data point for time t, series j.
  // This class needs easy access to w[j, t] because it is used in the Kalman
  // filter.
  //
  // The state alpha[t] has two components: shared state and series-specific
  // state.  A shared state component is a regular state component from a
  // dynamic factor model, with a matrix Z[t] mapping state to outcomes.  A
  // series specific model maintains a separate element of state for each
  // dimension of Y[t].
  //
  // The learning algorithm can cycle between (draw shared state given data and
  // series-specific state), (draw series-specific state), and (draw parameters
  // given complete data).
  //
  // The model assumes that errors from each state component are independent of
  // other state components (given model parameters), and that the observation
  // errors epsilon[t] are conditionally independent of everything else given
  // state and model parameters.  The state disturbance eta[t] is either
  // Gaussian or conditionally Gaussian.
  //
  // The independence assumptions outlined above mean the regression is handled
  // by nseries() separate student regression models.  Each can have its own
  // prior, which can be linked by a hierarchy.  If there is a model hierarchy,
  // it is to be maintained by the PosteriorSampler.
  //
  //---------------------------------------------------------------------------
  // The basic usage idiom is
  // NEW(StudentMvssRegressionModel, model)(xdim, ydim);
  // for() { model->add_data(data_point); }
  // model->add_state(shared_state_model_1);
  // model->add_state(shared_state_model_2);
  // ...
  // model->series_specific_model(0)->add_state(series_specific_state_model_11);
  // model->series_specific_model(0)->add_state(series_specific_state_model_12);
  // model->series_specific_model(1)->add_state(series_specific_state_model_21);
  // model->observation_model()->set_method(prior_for_regression_part);
  //
  // The posterior samplers for the individual state models must be set
  // separately, as must the sampler for the observation model.  If (e.g.) a
  // hierarchical regression is desired then that is a new posterior sampler
  // class for IndependentRegressionModels.
  class StudentMvssRegressionModel
      : public ConditionallyIndependentMultivariateStateSpaceModelBase,
        public PriorPolicy
  {
   public:
    friend class BoomStateSpaceTesting::StudentMvssRegressionModelTest;
    using Proxy = ProxyScalarStateSpaceModel<StudentMvssRegressionModel>;
    using ObservationModel = IndependentGlms<CompleteDataStudentRegressionModel>;

    // Args:
    //   xdim:  The dimension of the static regression component.
    //   nseries:  The number of time series being modeled.
    explicit StudentMvssRegressionModel(int xdim, int nseries);

    // This is a complex model with lots of subordinate parts.  Copying it
    // correctly would be really hard, so copying is disallowed.
    StudentMvssRegressionModel(
        const StudentMvssRegressionModel &rhs) = delete;
    StudentMvssRegressionModel &operator=(
        const StudentMvssRegressionModel &rhs) = delete;
    StudentMvssRegressionModel(
        StudentMvssRegressionModel &&rhs) = delete;
    StudentMvssRegressionModel &operator=(
        StudentMvssRegressionModel &&rhs) = delete;

    // An error will be reported if someone attempts to clone this model.
    StudentMvssRegressionModel *clone() const override;
    StudentMvssRegressionModel *deepclone() const override;

    // Simulate a multi-period forecast.
    // Args:
    //   rng:  The [0, 1) random number generator to use for the simulation.
    //   forecast_predictors: A matrix containing the predictor variables to use
    //     for the forecast.  The number of rows in the matrix is nseries() *
    //     forecast_horizon.  The rows are assumed to go (time0, series0),
    //     (time0, series1), ..., (time1, series0), (time1, series1), ...
    //   final_shared_state: The vector of shared state as of the final training
    //     data point at time_dimension() - 1.
    //   series_specific_final_state: This argument can be empty if there is no
    //     series-specific state in the model.  Otherwise, it must have length
    //     nseries(), where each element is the series specific state vector at
    //     time time_dimension() - 1 for the corresponding series.  Individual
    //     elements can be empty if there is no series-specific state for that
    //     series.
    //
    // Returns:
    //   A matrix containing draws of the next 'forecast_horizon' time periods.
    //   Each series corresponds to a row in the returned matrix, while columns
    //   correspond to time.  The simulation includes simulated residual error.
    Matrix simulate_forecast(
        RNG &rng,
        const Matrix &forecast_predictors,
        const Vector &final_shared_state,
        const std::vector<Vector> &series_specific_final_state);

    //------------------------------------------------------------------------
    // Access to state models.  Access to state comes from the
    // MultivariateStateSpaceModelBase "grandparent" base class.
    // ------------------------------------------------------------------------

    // Add state to the "shared-state" portion of the state space.
    void add_state(const Ptr<SharedStateModel> &state_model) {
      state_manager_.add_shared_state(state_model);
      set_parameter_observers(state_model.get());
    }

    // Add state to the state model for an individual time series.
    //
    // Args:
    //   state_model:  The state model defining the state to be added.
    //   series:  The index of the scalar time series described by the state.
    void add_series_specific_state(const Ptr<StateModel> &state_model,
                                   int series) {
      state_manager_.add_series_specific_state(state_model, series);
      set_parameter_observers(state_model.get());
    }

    // Indicates whether any of the proxy models have had state assigned.
    bool has_series_specific_state() const {
      return state_manager_.has_series_specific_state();
    }

    // Dimension of shared state.
    int state_dimension() const override {
      return state_manager_.shared_state_dimension();
    }

    // The dimension of the series-specific state associated with a particular
    // time series.
    int series_state_dimension(int which_series) const {
      return state_manager_.series_state_dimension(which_series);
    }

    int number_of_state_models() const override {
      return state_manager_.number_of_shared_state_models();
    }

    SharedStateModel *state_model(int s) override {
      return state_manager_.shared_state_model(s);
    }

    const SharedStateModel *state_model(int s) const override {
      return state_manager_.shared_state_model(s);
    }

    // Impute both the shared and series-specific state, each conditional on the
    // other.
    void impute_state(RNG &rng) override {
      MultivariateStateSpaceModelBase::resize_state();
      impute_student_weights(rng);
      impute_shared_state_given_series_state(rng);
      impute_series_state_given_shared_state(rng);
    }

    //-----------------------------------------------------------------------
    // Data policy overrides, and access to raw data.
    //-----------------------------------------------------------------------

    // The number of time points that have been observed.
    int time_dimension() const override {return data_policy_.time_dimension();}

    // The number of time series being modeled.
    int nseries() const override {return data_policy_.nseries();}

    // The dimension of the predictors.
    int xdim() const {return observation_model_->xdim();}

    // Adding data to this model adjusts time_dimension_, data_indices_, and
    // observed_.
    void add_data(const Ptr<Data> &dp) override {
      this->add_data(dp.dcast<StudentMultivariateTimeSeriesRegressionData>());
    }
    void add_data(const Ptr<StudentMultivariateTimeSeriesRegressionData> &dp) {
      data_policy_.add_data(dp);
    }

    void combine_data(const Model &rhs, bool just_suf = true) override {
      const StudentMvssRegressionModel *other_model =
          dynamic_cast<const StudentMvssRegressionModel *>(&rhs);
      if (other_model) {
        data_policy_.combine_data(other_model->data_policy_);
      } else {
        report_error("rhs could not be converted to "
                     "StudentMvssRegressionModel.");
      }
    }

    // An override is needed so model-specific meta-data can be cleared as well.
    void clear_data() override {data_policy_.clear_data();}

    int64_t data_index(int series, int time) {
      return data_policy_.data_index(series, time);
    }

    // Scalar data access.
    double response(int series, int time) const {
      int64_t index = data_policy_.data_index(series, time);
      if (index < 0) {
        return negative_infinity();
      } else {
        return data_policy_.data_point(index)->y();
      }
    }

    // A flag indicating whether a specific series was observed at time t.
    bool is_observed(int series, int time) const {
      return data_policy_.observed(time)[series];
    }

    // Vector data access.
    ConstVectorView observation(int t) const override {
      return data_policy_.observation(t);
    }

    const Selector &observed_status(int t) const override {
      return data_policy_.observed(t);
    }

    // Set the observation status for the data at time t.
    void set_observed_status(int t, const Selector &status) {
      data_policy_.set_observed_status(t, status);
    }

    // Returns the observed data point for the given series at the given time
    // point.  If that data point is missing, negative_infinity is returned.
    double observed_data(int series, int time) const {
      return response(series, time);
    }

    // The response value after contributions from "other models" have been
    // subtracted off.  It is the caller's responsibility to do the subtracting
    // (e.g. with isolate_shared_state() or isolate_series_specific_state()).
    double adjusted_observation(int series, int time) const {
      return adjusted_observation(time)[series];
    }

    // The vector of adjusted observations across all time series at time t.
    ConstVectorView adjusted_observation(int time) const override;

    //--------------------------------------------------------------------------
    // Kalman filter parameters.
    //--------------------------------------------------------------------------

    void isolate_shared_state() override {
      data_policy_.isolate_shared_state();
    }

    void isolate_series_specific_state() override {
      data_policy_.isolate_series_specific_state();
    }

    // The observation coefficients from the shared state portion of the model.
    // This does not include the regression coefficients from the regression
    // model, nor does it include the series-specific state.
    Ptr<SparseKalmanMatrix> observation_coefficients(
        int t, const Selector &observed) const override {
      return state_manager_.observation_coefficients(t, observed);
    }

    DiagonalMatrix observation_variance(int t) const override;
    DiagonalMatrix observation_variance(
        int t, const Selector &observed) const override;
    Vector observation_variance_parameter_values() const override;

    double single_observation_variance(
        int time, int which_series) const override;

    Proxy *series_specific_model(int index) {
      return state_manager_.series_specific_model(index);
    }

    const Proxy *series_specific_model(int index) const {
      return state_manager_.series_specific_model(index);
    }

    ObservationModel *observation_model() override {
      return observation_model_.get();
    }

    const ObservationModel *observation_model() const override {
      return observation_model_.get();
    }

    // The contribution of a particular state model to the mean of the response.
    //
    // Args:
    //   which_state_model:  The index of the desired state model.
    //
    // Returns:
    //   A matrix with rows corresponding to dimension of Y, and columns
    //   corresponding to time.
    Matrix state_contributions(int which_state_model) const override {
      return state_manager_.state_contributions(which_state_model, this);
    }

    StateSpaceUtils::StateModelVector<SharedStateModel>
    &state_models() override {
      return state_manager_.shared_state_models();
    }

    const StateSpaceUtils::StateModelVector<SharedStateModel>
    &state_models() const override {
      return state_manager_.shared_state_models();
    }

    // Ensure that all state and proxy models are aware of times up to time 't'.
    void observe_time_dimension(int t) {
      state_manager_.observe_time_dimension(t);
    }

    //------------- Parameter estimation by MLE and MAP --------------------
    // Set model parameters to their maximum-likelihood estimates, and return
    // the likelihood at the MLE.  Note that some state models cannot be used
    // with this method.  In particular, regression models with spike-and-slab
    // priors can't be MLE'd.  If the model contains such a state model then an
    // exception will be thrown.
    //
    // Args:
    //   epsilon: Convergence for optimization algorithm will be declared when
    //     consecutive values of log-likelihood are observed with a difference
    //     of less than epsilon.
    //
    // Returns:
    //   The value of the log-likelihood at the MLE.
    // double mle(double epsilon = 1e-5, int ntries = 500) override;

    // Returns true if all the state models have been assigned priors that
    // implement find_posterior_mode.
    // bool check_that_em_is_legal() const;

    // double Estep(bool save_state_distributions);
    // void Mstep(double epsilon);

    // Implements part of a single step of the E-step in the EM algorithm or
    // gradient computation for the gradient of the observed data log
    // likelihood.
    //
    // Args:
    //   r: Durbin and Koopman's r vector, which is a scaled version of the
    //     smoothed state mean.  On entry r is r[t].  On exit it is r[t-1].
    //   N: Durbin and Koopman's N matrix, which is a scaled version of the
    //     smoothed state variance. On entry N is N[t].  On exit it is N[t-1].
    //   t:  The time index for the update.
    //   save_state_distributions: If true then the observation error mean and
    //     variance (if y is univariate) or precision (if y is multivariate)
    //     will be saved in the Kalman filter.
    //   update_sufficient_statistics: If true then the complete data sufficient
    //     statistics for the observation model will be updated as in the E-step
    //     of the EM algorithm.
    //   gradient: If non-NULL then the observation model portion of the
    //     gradient will be incremented to reflect information at time t.
    //
    // Side effects:
    //   r and N are "downdated" to time t-1 throug a call to the disturbance
    //   smoother.  The Kalman filter is updated by the smoothing recursions.
    // void update_observation_model(Vector &r, SpdMatrix &N, int t,
    //                               bool save_state_distributions,
    //                               bool update_sufficient_statistics,
    //                               Vector *gradient);

    // Utility function used to implement E-step and log_likelihood_derivatives.
    //
    // Args:
    //   update_sufficient_statistics: If true then the complete data sufficient
    //     statistics for the observation model and the state models will be
    //     cleared and updated.  If false they will not be modified.
    //   save_state_distributions: If true then the state distributions (the
    //     mean vector a and the variance P) will be saved in kalman_storage_.
    //     If not then these quantities will be left as computed by the
    //     full_kalman_filter.
    //   gradient: If a nullptr is passed then no gradient information will be
    //     computed.  Otherwise the gradient vector is resized, cleared, and
    //     filled with the gradient of log likelihood.
    //
    // Returns:
    //   The log likelihood value computed by the Kalman filter.
    // double average_over_latent_data(bool update_sufficient_statistics,
    //                                 bool save_state_distributions,
    //                                 Vector *gradient);

    // A helper function used to implement average_over_latent_data().
    // Increments the gradient of log likelihood contribution of the state
    // models at time t (for the transition to time t+1).
    //
    // Args:
    //   gradient:  The gradient to be updated.
    //   t: The time index for the update.  Homogeneous models will
    //     ignore this, but models where the Kalman matrices depend on
    //     t need to know it.
    //   state_error_mean: The posterior mean of the state errors at
    //     time t (for the transition to time t+1).
    //   state_error_mean: The posterior variance of the state errors
    //     at time t (for the transition to time t+1).
    // void update_state_model_gradient(Vector *gradient, int t,
    //                                  const Vector &state_error_mean,
    //                                  const SpdMatrix &state_error_variance);


    // Increment the portion of the log-likelihood gradient pertaining to the
    // parameters of the observation model.
    //
    // Args:
    //   gradient: The subset of the log likelihood gradient pertaining to the
    //     observation model.  The gradient will be incremented by the
    //     derivatives of log likelihood with respect to the observation model
    //     parameters.
    //   t:  The time index of the observation error.
    //   observation_error_mean: The posterior mean of the observation error at
    //     time t.
    //   observation_error_variances: The posterior variance of the observation
    //     error at time t.
    void update_observation_model_gradient(
        VectorView gradient,
        int t,
        const Vector &observation_error_mean,
        const Vector &observation_error_variances) override {
      report_error("update_observation_model_gradient is not implemented.");
    }

    // Update the complete data sufficient statistics for the state models,
    // given the posterior distribution of the state error at time t (for the
    // transition between times t and t+1), given model parameters and all
    // observed data.
    //
    // Args:
    //   state_error_mean: The mean of the state error at time t given observed
    //     data and model parameters.
    //   state_error_variance: The variance of the state error at time t given
    //     observed data and model parameters.
    // void update_state_level_complete_data_sufficient_statistics(
    //     int t, const Vector &state_error_mean,
    //     const SpdMatrix &state_error_variance);

    // Update the complete data sufficient statistics for the observation model
    // based on the posterior distribution of the observation model error term
    // at time t.
    //
    // Args:
    //   t: The time of the observation.
    //   observation_error_mean: Mean of the observation error given model
    //     parameters and all observed y's.
    //   observation_error_variance: Variance of the observation error given
    //     model parameters and all observed y's.
    void update_observation_model_complete_data_sufficient_statistics(
        int t,
        const Vector &observation_error_mean,
        const Vector &observation_error_variances) override {
      report_error("update_observation_model_complete_data_sufficient_statistics "
                   "is not implemented.");
    }

    using ConditionallyIndependentMultivariateStateSpaceModelBase::get_filter;

    void impute_student_weights(RNG &rng);

   private:
    // Set observers on the variance parameters of the regression models, so
    // that the diagonal variance matrix can be updated when it gets out of
    // sync.
    void set_observation_variance_observers();

    // Ditto for adjusted_data_workspace_.
    void set_workspace_observers();

    // To be called during construction.  Set an observer on the parameters of
    // *model that will invalidate the current kalman filter likelihood
    // calculation when the parameters change.
    void set_parameter_observers(Model *model);

    // If the observation variance is out of step with the observation_variance_
    // data member, update the data member.  This function is logically const.
    void update_observation_variance() const;

    void resize_subordinate_state();
    void observe_state(int t) override;
    void observe_initial_state();
    void observe_data_given_state(int t) override;

    void impute_shared_state_given_series_state(RNG &rng);
    void impute_series_state_given_shared_state(RNG &rng);

    // The contribution of the series_specific state to the given series at the
    // given time.
    double series_specific_state_contribution(int series, int time) const {
      return state_manager_.series_specific_state_contribution(series, time);
    }

    //--------------------------------------------------------------------------
    // Data section.
    //--------------------------------------------------------------------------

    MultivariateStateSpaceRegressionDataPolicy<
      StudentMultivariateTimeSeriesRegressionData> data_policy_;

    // The number of series being modeled.
    // int nseries_;

    // The time dimension is the number of distinct time points.
    // int time_dimension_;

    StateSpaceUtils::SharedStateModelManager<Proxy> state_manager_;

    // The shared state models are stored in this container.  The series
    // specific state models are stored in proxy_models_.
    // StateSpaceUtils::StateModelVector<SharedStateModel> shared_state_models_;

    // The proxy models hold components of state that are specific to individual
    // data series.
    // std::vector<Ptr<Proxy>> proxy_models_;

    // data_indices_[series][time] gives the index of the corresponding element
    // of dat().
    //
    // NOTE: This is
    // std::map<int, std::map<int, int>> data_indices_;

    // The observation model.
    Ptr<ObservationModel> observation_model_;

    // A workspace to copy the residual variances stored in observation_model_
    // in the data structure expected by the model.
    mutable DiagonalMatrix observation_variance_;

    // A flag to keep track of whether the observation variance is current.
    mutable bool observation_variance_current_;

    // A Selector of size nseries() with all elements included.  Useful for
    // calling observation_coefficients when you want to assume all elements are
    // included.
    Selector dummy_selector_;
  };

}  // namespace BOOM

#endif  // BOOM_STUDENT_MULTIVARIATE_STATE_SPACE_REGRESSION_HPP_
