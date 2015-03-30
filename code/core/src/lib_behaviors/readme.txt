behavior name: bhv_waypoint_hover

Description: To reduce overshoot when starting on a new leg, this version of 
the waypoint behavior adds a heading cutoff above which the next waypoint will
be used instead of a trackpoint.  Once the heading cutoff is met, the 
trackpoint will be moved back along the trackline to its normal position.  

New configuration variables:
trackpoint_heading_cutoff - heading above which trackpoint will not be used.