#include <hi_freq_decay_generator.h>

#include <hi_freq_vertex_generator.h>

#include <genbb_help/primary_event.h>
#include <genbb_help/manager.h>

namespace hi_freq_sim {

  GENBB_PG_REGISTRATION_IMPLEMENT(hi_freq_decay_generator, "hi_freq_sim::hi_freq_decay_generator")


  double hi_freq_decay_generator::get_activity() const {
    return _activity_;
  }

  void hi_freq_decay_generator::set_activity(double activity_) {
    DT_THROW_IF(_initialized_, std::logic_error, "Operation prohibited ! Object is locked/initialized !");
    DT_THROW_IF(activity_ <= 0., std::logic_error, "Invalid activity value (>0) for particle generator '" << get_name() << "' !");
    _activity_ = activity_;
  }

  double hi_freq_decay_generator::get_readout_window_duration() const {
    return _readout_window_duration_;
  }

  void hi_freq_decay_generator::set_readout_window_duration(double duration_) {
    DT_THROW_IF(_initialized_, std::logic_error, "Operation prohibited ! Object is locked/initialized !");
    DT_THROW_IF(duration_ <= 0., std::logic_error, "Invalid readout duration (>0) for particle generator '" << get_name() << "' !");
    _readout_window_duration_ = duration_;
  }

  void hi_freq_decay_generator::set_event_generator_name(const std::string & eg_name_) {
    DT_THROW_IF(_initialized_, std::logic_error, "Operation prohibited ! Manager is locked !");
    _eg_name_ = eg_name_;
  }

  hi_freq_decay_generator::hi_freq_decay_generator() : genbb::i_genbb() {
    _initialized_ = false;
    _set_defaults();
  }

  hi_freq_decay_generator::~hi_freq_decay_generator() {
    if(_initialized_) {
      reset();
    }
  }

  bool hi_freq_decay_generator::is_initialized() const {
    return _initialized_;
  }

  void hi_freq_decay_generator::_set_defaults() {
    _activity_ = 0.;
    _seed_ = 0;
    _max_delay_time_ = -1.;
    _readout_window_duration_ = 0.;
  }

  bool hi_freq_decay_generator::can_external_random() const
  {
    return true;
  }

  const mygsl::rng & hi_freq_decay_generator::get_random() const
  {
    if (has_external_random()) {
      return get_external_random();
    }
    return *_random_;
  }

  mygsl::rng & hi_freq_decay_generator::grab_random()
  {
    if (has_external_random()) {
      return grab_external_random();
    }
    return *_random_;
  }

  void hi_freq_decay_generator::_at_reset_()
  {
    if (_random_) {
      if (_random_->is_initialized()) {
        _random_->reset();
      }
      _random_.reset();
    }
    _set_defaults();
    return;
  }

  void hi_freq_decay_generator::reset() {
    if(! _initialized_) {
      return;
    }
    _at_reset_();
    _initialized_ = false;
  }

  void hi_freq_decay_generator::initialize(const datatools::properties & config_,
                                             datatools::service_manager & service_manager_,
                                             detail::pg_dict_type & dictionary_)
  {
    DT_THROW_IF(_initialized_,std::logic_error, "Operation prohibited ! Object is already initialized !");

    _initialize_base(config_);

    if (! has_external_random()) {
      DT_THROW_IF(! config_.has_key("seed"), std::logic_error,
                  "Missing 'seed' property for particle generator '" << get_name() << "' !");
      long seed = config_.fetch_integer("seed");
      DT_THROW_IF(seed < 0, std::logic_error, "Invalid seed value (>=0) for particle generator '" << get_name() << "' !");
      _seed_ = seed;
    }

    DT_THROW_IF(!config_.has_key("activity"), std::logic_error,
                "Missing 'activity' property for particle generator '" << get_name() << "' !");
    {
      std::string unit_label;
      double unit_val;
      DT_THROW_IF(!config_.has_explicit_unit("activity") ||
          !datatools::units::find_unit(config_.get_unit_symbol("activity"), unit_val, unit_label) ||
          unit_label != "activity", std::logic_error,
          "Missing 'activity' property with an activity unit for particle generator '" << get_name() << "' !");
    }
    double activity = config_.fetch_real("activity");
    DT_THROW_IF(activity <= 0., std::logic_error, "Invalid activity value (>0) for particle generator '" << get_name() << "' !");
    set_activity(activity);
    DT_LOG_TRACE(_logging_priority, "Activity is '" << _activity_ << "'... or in Bq: " << _activity_ / CLHEP::becquerel );

    DT_THROW_IF(!config_.has_key("duration"), std::logic_error,
                "Missing 'duration' property for particle generator '" << get_name() << "' !");
    {
      std::string unit_label;
      double unit_val;
      DT_THROW_IF(!config_.has_explicit_unit("duration") ||
          !datatools::units::find_unit(config_.get_unit_symbol("duration"), unit_val, unit_label) ||
          unit_label != "time", std::logic_error,
          "Missing 'duration' property with a time unit for particle generator '" << get_name() << "' !");
    }
    double dur = config_.fetch_real("duration");
    DT_THROW_IF(activity <= 0., std::logic_error, "Invalid readout duration  (>0) for particle generator '" << get_name() << "' !");
    set_readout_window_duration(dur);
    DT_LOG_TRACE(_logging_priority, "Readout duration is '" << _readout_window_duration_ << "'... or in seconds: " << _readout_window_duration_ / CLHEP::second);

    DT_THROW_IF(!config_.has_key("event_generator_name"), std::logic_error,
                "Missing 'event_generator_name' property for particle generator '" << get_name() << "' !");
    const std::string eg_name = config_.fetch_string("event_generator_name");
    set_event_generator_name(eg_name);
    DT_LOG_TRACE(_logging_priority, "Event generator name is '" << _eg_name_ << "'...");

    genbb::detail::pg_dict_type::iterator found = dictionary_.find(_eg_name_);
    DT_THROW_IF(found == dictionary_.end(), std::logic_error,
                     "No particle generator named '" << _eg_name_ << "' !");
    genbb::detail::pg_entry_type & pet = found->second;
    if (! pet.is_initialized()) {
      datatools::factory_register<i_genbb> * facreg = 0;
      // Check if a manager can provide a factory object :
      if (pet.has_manager()) {
        facreg = &pet.grab_manager().grab_factory_register();
      }
      detail::initialize(pet,
          &service_manager_,
          &dictionary_,
          facreg,
          (has_external_random()? &grab_external_random() : 0));
    } // if (! pet.is_initialized()) {
    //pet.tree_dump(std::cerr, "genbb::combined_particle_generator::initialize: PET: ", "DEVEL: ");
    _event_generator_ = &(pet.grab());

    _at_init_();

    _initialized_ = true;
  }

  bool hi_freq_decay_generator::has_next ()
  {
    return true;
  }

  void hi_freq_decay_generator::_load_next (::genbb::primary_event & event_,
                                              bool compute_classification_)
  {
    DT_LOG_TRACE(get_logging_priority(),"Entering...");
    DT_THROW_IF(! _initialized_, std::logic_error,
                 "Generator is not initialized !");
    event_.reset();

    if(_max_delay_time_ < 0.) {
      // running for the first time;
      // first generate 100x the expected decays in a single readout window, to get maximum expected decay at some precision
      long n_to_gen = (long)(100. * _activity_ / CLHEP::becquerel * _readout_window_duration_ / CLHEP::second);
      DT_LOG_TRACE(get_logging_priority(),"Num to gen initially = "<<n_to_gen);
      double max_del_t = 0.;
      primary_event an_event;
      for(long i = 0; i < n_to_gen; ++i) {
        _event_generator_->load_next(an_event, false);
        double mdt = 0.;
        for (size_t ip = 0; ip < an_event.get_number_of_particles(); ++ip) {
          primary_particle & pp = an_event.grab_particle(ip);
          if(pp.get_time() > mdt) mdt = pp.get_time();
        }
        if(mdt > max_del_t) {
          max_del_t = mdt;
        }
      }
      _max_delay_time_ = max_del_t;
      DT_LOG_TRACE(get_logging_priority(),"Calculated max delay as "<<_max_delay_time_<<" ns");

      if(_max_delay_time_ > 0.) {
        n_to_gen = grab_random().poisson(_activity_ / CLHEP::becquerel * _max_delay_time_ / CLHEP::second);
        DT_LOG_TRACE(get_logging_priority(),"Num to generate before start = "<<n_to_gen);


        std::vector<double> pre_decay_times;
        for(long i = 0; i < n_to_gen; ++i) {
          pre_decay_times.push_back(grab_random().flat(-_max_delay_time_, 0.));

        }
        //std::sort(pre_decay_times.begin(), pre_decay_times.end());


        geomtools::vector_3d vertex;
        for(long i = 0; i < n_to_gen; ++i) {
          _event_generator_->load_next(an_event, false);
          bool has_vertex = false;
          for (size_t ip = 0; ip < an_event.get_number_of_particles(); ++ip) {
            primary_particle & pp = an_event.grab_particle(ip);
            double t = pp.get_time();
            if(t + pre_decay_times[i] > 0.) {
              if(!has_vertex) {
                hi_freq_sim::hi_freq_vertex_generator::get_vg()->actually_shoot_vertex(vertex);
                has_vertex = true;
              }
              pp.set_vertex(vertex);
              pp.shift_time(pre_decay_times[i]);
              _timed_particles_.insert(std::pair<double, genbb::primary_particle>(t+pre_decay_times[i], pp));
            }
          }
        }
      }
      _current_global_time_ = 0.;

    }
    
    DT_LOG_TRACE(get_logging_priority(),"Current global time "<<_current_global_time_<<" ns");

    const long n_to_gen = grab_random().poisson(_activity_ / CLHEP::becquerel * _readout_window_duration_ / CLHEP::second);
    DT_LOG_TRACE(get_logging_priority(),"Num to gen this time = "<<n_to_gen);
    std::vector<double> decay_times;
    for(long i = 0; i < n_to_gen; ++i) {
      decay_times.push_back(grab_random().flat(_current_global_time_, _current_global_time_+_readout_window_duration_));
    }
    //std::sort(decay_times.begin(), decay_times.end());
    geomtools::vector_3d vertex;
    primary_event an_event;
    for(long i = 0; i < n_to_gen; ++i) {
      _event_generator_->load_next(an_event, false);
      bool has_vertex = false;
      for (size_t ip = 0; ip < an_event.get_number_of_particles(); ++ip) {
        primary_particle & pp = an_event.grab_particle(ip);
        double t = pp.get_time();
        if(!has_vertex) {
          hi_freq_sim::hi_freq_vertex_generator::get_vg()->actually_shoot_vertex(vertex);
          has_vertex = true;
        }
        pp.set_vertex(vertex);
        pp.shift_time(decay_times[i]);
        _timed_particles_.insert(std::pair<double, genbb::primary_particle>(t+decay_times[i], pp));
      }
    }

    double earliest_t = _timed_particles_.begin()->first;
    double latest_t = earliest_t + _readout_window_duration_;
    DT_LOG_TRACE(get_logging_priority(),"Earliest t = "<<earliest_t);
    DT_LOG_TRACE(get_logging_priority(),"Latest t = "<<latest_t);

    std::multimap<double, ::genbb::primary_particle>::iterator last_pp = _timed_particles_.begin();
    while(last_pp != _timed_particles_.end() && last_pp->second.get_time() < latest_t) {
      genbb::primary_particle& pp = last_pp->second;
      pp.shift_time(-earliest_t);
      event_.add_particle(pp);
      ++last_pp;
    }
    if(last_pp != _timed_particles_.begin()) {
      if(last_pp != _timed_particles_.end()) { ++last_pp; }
      _timed_particles_.erase(_timed_particles_.begin(), last_pp);
    }
    DT_LOG_TRACE(get_logging_priority(),"Number generated = "<<event_.get_particles().size());

    event_.set_label(genbb::i_genbb::get_name());


    _current_global_time_ = latest_t;

    if (compute_classification_) {
      event_.compute_classification();
    }
    DT_LOG_TRACE(get_logging_priority(),"Exiting.");
    
  }

  void hi_freq_decay_generator::_at_init_()
  {
    if (! has_external_random()) {
      _random_.reset(new mygsl::rng);
      _random_->init("taus2", _seed_);
    }

  }

} // end namespace hi_freq_sim
