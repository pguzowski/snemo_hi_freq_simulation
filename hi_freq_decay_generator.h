#ifndef HI_FREQ_SIMULATION_HI_FREQ_DECAY_GENERATOR
#define HI_FREQ_SIMULATION_HI_FREQ_DECAY_GENERATOR

#include <string>
#include <map>
// - Boost:
#include <boost/scoped_ptr.hpp>
// - Bayeux/datatools:
#include <datatools/properties.h>
// - Bayeux/mygsl:
#include <mygsl/rng.h>
// This project:
#include <genbb_help/i_genbb.h>
#include <genbb_help/primary_particle.h>


//namespace genbb {
//  class primary_particle;
//}


namespace hi_freq_sim {

  class hi_freq_decay_generator : public genbb::i_genbb {
    public:

      double get_activity() const;
      void set_activity(double);

      double get_readout_window_duration() const;
      void set_readout_window_duration(double);

      /// Set the name of the active event generator
      void set_event_generator_name(const std::string &);

      /// Check existence of external random
      virtual bool can_external_random() const;

      /// Return a non-mutable random generator
      const mygsl::rng & get_random() const;

      /// Return a mutable random generator
      mygsl::rng & grab_random();

      //Constructor
      hi_freq_decay_generator();

      virtual ~hi_freq_decay_generator();

      virtual void initialize(const datatools::properties & setup_,
                             datatools::service_manager & service_manager_,
                             detail::pg_dict_type & dictionary_);

      virtual void reset();

      virtual bool has_next();

      virtual bool is_initialized() const;

    protected:

      virtual void _load_next(primary_event & event_,
                              bool compute_classification_ = true);

      void _set_defaults();

    private:

      void _at_init_();

      void _at_reset_();

    private:

      bool _initialized_;  //!< Initialization flag

      double _activity_;  //!< Activity of the decay
      double _max_delay_time_;
      double _readout_window_duration_;
      double _current_global_time_;

      std::string      _eg_name_;         //!< Name of the active event generator
      ::genbb::i_genbb  *  _event_generator_; //!< The external event generator

      std::multimap<double, ::genbb::primary_particle> _timed_particles_; //!< storage of all generated particles ordered by time

      std::map<std::string, double> _decay_halflives_; //!< storage of maximum expected delayed tracks for a decay

      unsigned long _seed_;   //!< PRNG seed (local or global)
      boost::scoped_ptr<mygsl::rng>    _random_; //!< Local PRNG

      GENBB_PG_REGISTRATION_INTERFACE(hi_freq_decay_generator)
  };
} // end of namespace hi_freq_sim

#endif // HI_FREQ_SIMULATION_HI_FREQ_DECAY_GENERATOR
