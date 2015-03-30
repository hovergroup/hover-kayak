driver posts transmissions to ACOMMS_SIM_IN
    format: $VNAME,ModemTransmissionProtobuf

driver posts periodic reports to ACOMMS_SIM_REPORT
    format: AcommsSimReportProtobuf
    published every second

simulator posts output to ACOMMS_SIM_OUT_$VNAME
    format: ModemTransmissionProtobuf for receptions
    format: "   $$$" where $$$ is the 3 letter raw message
    
possible raw messages:
    TXF - finish transmission
    RXP - receive start