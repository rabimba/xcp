#include "qdelay-estimator.hh"
#include <assert.h>
#include <stdio.h>

const double QdelayEstimator::get_ts(const Packet* p) {
	return hdr_cmn::access(p)->timestamp();
}

const uint32_t QdelayEstimator::get_seq(const Packet* p) {
	return hdr_cmn::access(p)->uid();
}

const double QdelayEstimator::get_service_time(const Packet* p) {
	return hdr_cmn::access(p)->size()*8.0/_link_rate;
}

QdelayEstimator::QdelayEstimator(PacketQueue* pkts, double link_rate) :
	_pkts( std::queue<const Packet*>() ),
	_link_rate(link_rate),
	_delays(std::vector<double> ())
{
	pkts->resetIterator();
	Packet *p = pkts->getNext();
	while (p != nullptr) {
		_pkts.push(p);
		p = pkts->getNext();
	}
	pkts->resetIterator();
	assert( !_pkts.empty() );
}

std::vector<double> QdelayEstimator::estimate_delays(double now)
{
	/* Base case(W_0) in Lindley's recurrence */
	const Packet* hol_pkt = _pkts.front();
	_pkts.pop();
	double current_delay = now - get_ts(hol_pkt) + get_service_time(hol_pkt);
	_delays.push_back( current_delay );
	const Packet* previous_pkt = hol_pkt;

	/* Apply Lindley's recurrence using constant service time */
        const Packet* current_pkt = nullptr;
	while ( !_pkts.empty() ) {
		const Packet* current_pkt = _pkts.front();
		_pkts.pop();
		double inter_arrival_time = get_ts(current_pkt) - get_ts(previous_pkt);
		double service_time = get_service_time(current_pkt);
		assert( inter_arrival_time >= 0 );
		current_delay = std::max(current_delay + service_time - inter_arrival_time, 0.0);
		assert( current_delay >= 0 );
		_delays.push_back( current_delay );
		previous_pkt = current_pkt ;
	}
	/* return delays */
	return _delays;
}
