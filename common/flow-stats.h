#ifndef FLOW_STATS_HH
#define FLOW_STATS_HH

#include "ewma-estimator.h"
#include "packet.h"

/*
   Book keeping information
   for flow statistics estimation
 */
class FlowStats {
  private :
    double    _first_arrival;
    double    _first_service;
    double    _acc_arrivals;
    double    _acc_services;
    uint32_t  _arrival_count;
    uint32_t  _service_count;

  public :
    EwmaEstimator _arr_est;
    EwmaEstimator _ser_est;
    EwmaEstimator _link_est;
    EwmaEstimator _delay_est;
    double _K;

    /* Constructor */
    FlowStats( double K );

    /* Estimate arrival rate */
    double est_arrival_rate( double now, Packet* p );

    /* Estimate service rate */
    double est_service_rate( double now, Packet* p );

    /* Estimate link rate */
    double est_link_rate( double now, double current_link_rate );

    /* Estimate delay */
    double est_delay( double now, double current_delay );

    /* Get packet size */
    uint32_t get_pkt_size( Packet* p );

    /* Print rates */
    void print_rates(uint32_t flow_id, double now) const;

    /* Get current estimate of all quantities */
    double arr_rate(void)  const { return _arr_est.get_estimate();}
    double ser_rate(void)  const { return _ser_est.get_estimate();}
    double link_rate(void) const { return _link_est.get_estimate();}
    double get_delay(void) const { return _delay_est.get_estimate();}

};

#endif
