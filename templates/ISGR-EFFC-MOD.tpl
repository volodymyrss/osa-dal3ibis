################################################################################
#
#DATANAME IBIS-ISGRI - GAIN OFFSET CORRECTION - INSTRUMENT MODEL
#
#DESCRIPTION
# Contains the amplitude and risetime gain offset correction for
# the IBIS-ISGRI detector layer.
#
#TEMPLATE ISGR-OFFS-MOD
#
#CONTENT BINARY TABLE
#
#RP ic
#DB No
#V1
#
#CHANGES
# 1.5   : Template creation
# 2.0.1 : Added the keywords needed for instrum. charact. files:
#       : VERSION, FILENAME, LOCATN, RESPONSI, STRT_VAL, END_VAL,
#       : VSTART, VSTOP, MJDREF, TIMESYS, TIMEUNIT and TIMEREF
# 3.3   : Template put under Configuration Control
# 4.0   : Changed column formats to 1E and added PIXTYPE column (SCREW-00519)
#
################################################################################
XTENSION	BINTABLE	/ Binary table extension
EXTNAME		ISGR-EFFC-MOD	/ Extension name
EXTREL		'4.0'		/ ISDC release number
BASETYPE	DAL_TABLE	/ Data Access Layer base type
TELESCOP	INTEGRAL	/ Telescope or mission name
ORIGIN		String		/ Origin of FITS file
INSTRUME	IBIS		/ Instrument name
DETNAM		ISGRI		/ Name of the detector layer
CREATOR		String		/ Program that created this FITS file
CONFIGUR	String		/ Software configuration
DATE		UTC_format	/ FITS file creation date
MJDREF		51544.0		/ Modified Julian Date of time origin
TIMESYS		TT		/ Time frame system
TIMEUNIT	d		/ Time unit
TIMEREF		LOCAL		/ Time reference frame
VERSION		Integer		/ Version of the instrument characteristic file
FILENAME	String		/ Name of the instrument characteristic file
LOCATN		String		/ Site or institute delivering this file
RESPONSI	String		/ E-mail address of person in charge of delivery
STRT_VAL	UTC_format	/ Start of validity time in UTC
END_VAL		UTC_format	/ End of validity time in UTC (forever if empty)
VSTART		Real		/ Start of validity time in IJD
VSTOP		Real		/ End of validity time in IJD
	TTYPE#	PIXEL_GROUPING	/ Amplitude Gain
	TFORM#	1I		/ Format of column AGAIN
	TTYPE#	PIXEL_GROUP	/ Amplitude Gain
	TFORM#	1I		/ Format of column AGAIN
	TTYPE#	EFFICIENCY		/ Amplitude Offset
	TFORM#	256D		/ Format of column AOFFSET
