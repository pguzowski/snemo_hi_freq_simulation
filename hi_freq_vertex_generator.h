#ifndef HI_FREQ_SIMULATION_HI_FREQ_VERTEX_GENERATOR
#define HI_FREQ_SIMULATION_HI_FREQ_VERTEX_GENERATOR

// This project:
#include <genvtx/vg_macros.h>
#include <genvtx/utils.h>

namespace hi_freq_sim {

  class hi_freq_vertex_generator : public genvtx::i_vertex_generator {
    public:
      /// Constructor
      hi_freq_vertex_generator();

      /// Destructor
      virtual ~hi_freq_vertex_generator();

      /// Check initialization status
      virtual bool is_initialized() const;

      /// Main initialization interface method
      virtual void initialize(const datatools::properties & setup_,
          datatools::service_manager & service_manager_,
          vg_dict_type & dictionary_);

      /// Reset method
      virtual void reset();

      virtual void actually_shoot_vertex(::geomtools::vector_3d & vertex_);

      static hi_freq_vertex_generator* get_vg();

    protected:
      virtual void _shoot_vertex(::mygsl::rng & random_,
                               ::geomtools::vector_3d & vertex_);


      /// Private reset
      void _reset_ ();

      /// Set default attributes' values
      void _set_defaults_ ();
      
    private:
      bool _initialized_; //!< Initialization flag
      genvtx::vg_handle_type vg_handle; //!< actual vertex generator
      mygsl::rng *   _random_;

      static hi_freq_vertex_generator* _vg_;


      GENVTX_VG_REGISTRATION_INTERFACE(hi_freq_vertex_generator)
  };

} // end namespace hi_freq_sim

#endif // HI_FREQ_SIMULATION_HI_FREQ_VERTEX_GENERATOR
