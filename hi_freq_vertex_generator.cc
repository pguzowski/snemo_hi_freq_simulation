#include <hi_freq_vertex_generator.h>


namespace hi_freq_sim {

  GENVTX_VG_REGISTRATION_IMPLEMENT(hi_freq_vertex_generator, "hi_freq_sim::hi_freq_vertex_generator")

  hi_freq_vertex_generator* hi_freq_vertex_generator::_vg_ = 0;

  hi_freq_vertex_generator* hi_freq_vertex_generator::get_vg() {
    DT_THROW_IF(_vg_ == 0, std::runtime_error, "hi_freq_vertex_generator has not been created yet!");
    return _vg_;
  }

  bool hi_freq_vertex_generator::is_initialized () const
  {
    return _initialized_;
  }

  hi_freq_vertex_generator::hi_freq_vertex_generator() : genvtx::i_vertex_generator()
  {
    _initialized_ = false;
    _set_defaults_ ();
  }

  hi_freq_vertex_generator::~hi_freq_vertex_generator()
  {
    if (is_initialized ()) reset ();
    return;
  }

  void hi_freq_vertex_generator::_set_defaults_ ()
  {
    this->i_vertex_generator::_reset ();
  }

  void hi_freq_vertex_generator::_reset_ ()
  {
    _set_defaults_ ();
  }

  void hi_freq_vertex_generator::reset ()
  {
    DT_THROW_IF (! is_initialized (), std::logic_error, "Vertex generator '" << get_name() << "' is not initialized !");
    _reset_ ();
    _initialized_ = false;
  }

  void hi_freq_vertex_generator::_shoot_vertex(::mygsl::rng & random_,
                                  ::geomtools::vector_3d & vertex_)
  {
    DT_THROW_IF (! is_initialized (), std::logic_error, "Vertex generator '" << get_name() << "' is not initialized !");
    _random_ = &random_;
    geomtools::invalidate (vertex_);
  }

  void hi_freq_vertex_generator::actually_shoot_vertex (geomtools::vector_3d & vertex_)
  {
    genvtx::i_vertex_generator & a_vg = vg_handle.grab ();
    DT_THROW_IF (! a_vg.has_next_vertex (),
                 std::logic_error,
                 "Vertex generator '" << a_vg.get_name() << "' has no more available vertex !");
    a_vg.shoot_vertex (*_random_, vertex_);
  }

  void hi_freq_vertex_generator::initialize (const ::datatools::properties & setup_,
                                ::datatools::service_manager & service_manager_,
                                ::genvtx::vg_dict_type & vgens_)
  {
    using namespace std;
    DT_THROW_IF (is_initialized (), std::logic_error, "Already initialized !");
    this->::genvtx::i_vertex_generator::_initialize_basics(setup_, service_manager_);
    this->::genvtx::i_vertex_generator::_initialize_geo_manager(setup_,service_manager_);
    // Parsing configuration parameters :
    DT_THROW_IF (! setup_.has_key ("vg_name"),
        std::logic_error,
        "Missing the 'vg_name' directive in combined vertex generator '" << get_name() << "' !");
    // extract information for combined generators :
    string vg_name = setup_.fetch_string("vg_name");

    DT_LOG_TRACE (get_logging_priority(), " VG name = '" << vg_name << "'");
    genvtx::vg_dict_type::iterator found = vgens_.find (vg_name);
    DT_THROW_IF (found == vgens_.end (), std::logic_error,
        "No vertex generator named '" << vg_name << "' in combined vertex generator '" << get_name() << "' !");
    genvtx::vg_handle_type vgh = found->second.grab_initialized_vg_handle ();

    vg_handle = vgh;

    _vg_ = this;
    _initialized_ = true;
    return;
  }
} // end namespace hi_freq_sim
