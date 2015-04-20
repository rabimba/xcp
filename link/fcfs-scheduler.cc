#include <algorithm>
#include "link/fcfs-scheduler.h"
#include "link/fcfs-tx-timer.h"

static class FcfsSchedulerClass : public TclClass {
 public :
  FcfsSchedulerClass() : TclClass("FcfsScheduler") {}
  TclObject* create(int argc, const char*const* argv) {
    return (new FcfsScheduler(atof(argv[4]), atoi(argv[5]), atof(argv[6])));
  }
} class_fcfs;

FcfsScheduler::FcfsScheduler(double rate_est_time_constant, uint32_t num_users, double feedback_delay)
    : EnsembleScheduler(rate_est_time_constant,
                        num_users,
                        feedback_delay),
      tx_timer_(new FcfsTxTimer(this)),
      user_service_rate_est_(std::vector<FlowStats> (num_users, FlowStats(rate_est_time_constant)))
{}

uint32_t FcfsScheduler::pick_user_to_schedule(void) const {
  /* Get feasible users */
  std::vector<uint32_t> feasible_users = get_feasible_users();

  /* Get user timestamps */
  std::vector<double> hol_ts( num_users_ );
  for (uint32_t i=0; i < num_users_ ; i++ ) {
    hol_ts.at(i) = user_queues_.at(i)->get_hol();
  }
  
  /* Pick the earliest ts amongst them */
  auto it = std::max_element(feasible_users.begin(), feasible_users.end(),
                             [&] (const uint32_t &f1, const uint32_t &f2)
                             { return ((Scheduler::instance().clock() - hol_ts.at(f1))*get_link_rate_estimate(f1)) < ((Scheduler::instance().clock() - hol_ts.at(f2)) * get_link_rate_estimate(f2));});

  return (it!=feasible_users.end()) ? *it : (uint32_t)-1;
}

int FcfsScheduler::command(int argc, const char*const* argv) {
  if (argc == 2) {
    if ( strcmp(argv[1], "activate-link-scheduler" ) == 0 ) {
      /* generate rates to start with */
      update_link_rate_estimate();
      /* transmit packet if it exists */
      transmit_pkt();
      return TCL_OK;
    }
  }
  return EnsembleScheduler::command(argc, argv);
}

void FcfsScheduler::transmit_pkt() {
  /* Update link rate estimate */
  update_link_rate_estimate();

  /* Get chosen user */
  uint32_t chosen_user = pick_user_to_schedule();

  /* If no one was scheduled, return */
  if (chosen_user==(uint32_t)-1) {
    busy_ = false;
    return;
  }

  /* If link rate is zero, return */
  if (user_links_.at(chosen_user)->bandwidth()==0) {
    busy_ = false;
    /* Wake up in 1ms or when SFD::enque wakes you up */
    tx_timer_->resched(FALLBACK_INTERVAL);
    return;
  }

  /* Get one packet from chosen user */
  Packet *p = user_queues_.at(chosen_user)->deque();
  assert(p!=nullptr);

  /* Get queue_handler */
  auto queue_handler = &user_queues_.at(chosen_user)->qh_;

  /* Get transmission time */
  double txt = user_links_.at(chosen_user)->txtime(p);

  /* Send packet onward */
  user_links_.at(chosen_user)->recv(p, queue_handler);
  busy_ = true;

  /* Log */
//  printf(" FcfsTxTimer::expire, Chosen_user %d, recving %f bits @ %f \n",
//         chosen_user,
//         user_links_.at(chosen_user)->bandwidth()*txt,
//         Scheduler::instance().clock());

  /* schedule next packet transmission */
  tx_timer_->resched(txt);

  /* Estimate service rate */
  user_service_rate_est_.at(chosen_user).est_service_rate(Scheduler::instance().clock(), p);

  /* Estimate aggregate service rate */
  agg_service_rate_est_.est_service_rate(Scheduler::instance().clock(), p);
}

void FcfsScheduler::reactivate_link() {
  transmit_pkt();
}
