# Phase parameters:
ASCPHASETIMINGPLAN              =   "1.3.6.1.4.1.1206.3.5.2.1.2.1.1"
CUR_TIMING_PLAN                 =   "1.3.6.1.4.1.1206.3.5.2.1.22.0"     
PHASE_PARAMETERS_MIN_GRN        =   "1.3.6.1.4.1.1206.3.5.2.1.2.1.9."   # need last "x.p" x is the timing plan number,p is the phase number: x get from CUR_TIMING_PLAN
PHASE_PARAMETERS_MAX_GRN        =   "1.3.6.1.4.1.1206.3.5.2.1.2.1.15."
PHASE_PARAMETERS_RED_CLR        =   "1.3.6.1.4.1.1206.3.5.2.1.2.1.19."
PHASE_PARAMETERS_YLW_XGE        =   "1.3.6.1.4.1.1206.3.5.2.1.2.1.18."

#**********asc3PhaseStatusTiming
#T (1):       Phase Timing
#N (2):       Phase Next
#- (3):       Phase Not Enabled
#(space) (4): Phase Not Timing or Next
PHASE_TIMING_STATUS             =   "1.3.6.1.4.1.1206.3.5.2.1.18.1.1."  #NEED last "p"  for the phase
#**********asc3PhaseStatusTiming2
# (1) X: XPED timing
# (2) N: Phase Next
# (3) -: Phase Not enabled
# (4) .: Phase Not Timing
# (5) R: Phase Timing RED
# (6) Y: Phase Timing YEL
# (7) G: Phase Timing GREEN

# (8) D: Phase Timing DELAY GREEN
# (9) O: Phase Timing YEL & RED
#(10) g: Phase Timing FLASHING GREEN
#(11) y: Phase Timing FLASHING YELLOW "
PHASE_TIMING_STATUS2            =   "1.3.6.1.4.1.1206.3.5.2.1.18.1.6."  #NEED last "p"  for the phase
asc3ViiMessageEnable	= "1.3.6.1.4.1.1206.3.5.2.9.44.1.1"
