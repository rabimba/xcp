// -*-  Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*-



#ifndef ns_xcp_end_sys_h
#define ns_xcp_end_sys_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "ip.h"
#include "tcp.h"
#include "flags.h"

#include "agent.h"
#include "packet.h"

#include "flags.h"
#include "tcp-sink.h"
#include "tcp-full.h"

#define XCP_HDR_LEN  20 // to match the internet draft 

struct hdr_xcp {
	double  x_;			//idealized inter-packet time
	double	rtt_;
	enum {
		XCP_DISABLED = 0,
		XCP_ENABLED,
		XCP_ACK,
	} 	xcp_enabled_;		// to indicate that the flow is XCP enabled
	int	xcpId_;			// Sender's ID (debugging only)
	double	cwnd_;			// The current window (debugging only) 
	double	reverse_feedback_;

	// --- Initialized by source and Updated by Router 
	double delta_throughput_;
	unsigned int controlling_hop_;  // The AQM ID of the controlling router

	static int offset_;	    // offset for this header
	inline static int& offset() { return offset_; }
	inline static hdr_xcp* access(Packet* p) {
		return (hdr_xcp*) p->access(offset_);
	}

	/* per-field member functions */
	double& cwnd() { return (cwnd_); }
	double& rtt() { return (rtt_); }
};


#define		MAX(a,b)	((a) > (b) ? (a) : (b))
#define		TP_TO_TICKS	MAX(1, (t_srtt_ >> T_SRTT_BITS))

#define TP_AVG_EXP		4	// used for xcp_metered_output_ == true

//Base class for Tcp and FullTcp XCP agents
class XcpEndsys {
protected:
	XcpEndsys(TcpAgent* tcp);
	void trace_var(const char *var_name, double var) const;
	void opencwnd() { /* nothing, cwnd is conrolled in recv() */ }
	void rtt_update(double tao);
	void init_rtt_vars() { srtt_estimate_ = 0.0; }
	void rtt_init();
	void recv(Packet *);
	void send(Packet *, int datalen);

	TcpAgent *tcp_;

	double	xcp_rev_fb_;	/* Accumulated throughput change to send back, B/s */
	double	current_positive_feedback_ ;
	int	tcpId_;
	double	srtt_estimate_;
	long	xcp_srtt_; // srtt estimate using the above macros

	/* more bits in delta for better precision, just for SRTT */
#define	XCP_DELTA_SHIFT		5
#define XCP_EXPO_SHIFT		3
#define	XCP_RTT_SHIFT		(XCP_DELTA_SHIFT + XCP_EXPO_SHIFT)	
	/* macros for SRTT calculations */
#define XCP_INIT_SRTT(rtt)					\
	((rtt) << XCP_RTT_SHIFT)
	   
#define XCP_UPDATE_SRTT(srtt, rtt)				\
	((srtt) + (((rtt) << XCP_DELTA_SHIFT)			\
		   - (((srtt) + (1 << (XCP_EXPO_SHIFT - 1)))	\
		      >> XCP_EXPO_SHIFT)))
	static unsigned int next_xcp_;
};

class XcpNewRenoFullTcpAgent : public NewRenoFullTcpAgent,
			       public XcpEndsys {
public:
	XcpNewRenoFullTcpAgent();

protected:
	/*New*/
	virtual void delay_bind_init_all();
	virtual int delay_bind_dispatch(const char *varName, 
					const char *localName, 
					TclObject *tracer);
	/*New*/
	virtual void opencwnd() { XcpEndsys::opencwnd(); }
	virtual void recv(Packet *, Handler *); 
	virtual void sendpacket(int seq, int ack, int flags, int dlen, int why, Packet *p=0);

	virtual void rtt_init(); // called in reset()
	virtual void rtt_update(double);
};

class XcpRenoTcpAgent : public RenoTcpAgent,
			public XcpEndsys {
public:
	XcpRenoTcpAgent();
protected:
	virtual void delay_bind_init_all();
	virtual int delay_bind_dispatch(const char *varName, 
					const char *localName, 
					TclObject *tracer);
        virtual void output_helper(Packet *);
        virtual void recv_newack_helper(Packet *);
        virtual void opencwnd() { XcpEndsys::opencwnd(); }
        virtual void rtt_init(); // called in reset()
        virtual void rtt_update(double);
};

class XcpTcpSink : public TcpSink,
		public XcpEndsys {
public:
	XcpTcpSink(Acker *);
	virtual void recv(Packet* pkt, Handler*);
protected:
        virtual void add_to_ack(Packet*);
        virtual void delay_bind_init_all();
        virtual int delay_bind_dispatch(const char *varName,
					const char *localName,
					TclObject *tracer);
};
	
#endif /* ns_xcp_end_sys_h */
