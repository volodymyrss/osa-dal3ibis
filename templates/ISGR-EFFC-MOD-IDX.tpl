################################################################################
#
#DATANAME IBIS - 2-DIMENSIONAL LOOK-UP TABLE 2 - INSTRUMENT MODEL - INDEX
#
#DESCRIPTION
# Index of all ISGR-LUT2-MOD data structures.
#
#TEMPLATE ISGR-LUT2-MOD-IDX
#
#CONTENT GROUP
#MEMBER ISGR-LUT2-MOD
#
#RP idx/ic
#DB No
#V1
#
#CHANGES
# 8.3   : Template creation (SCREW-01889)
#
################################################################################
\GROUP			ISGR-EFFC-MOD-IDX
	IDXMEMBR	ISGR-EFFC-MOD	/ Extname of the members of the index
	EXTREL		'8.3'		/ ISDC release number
	BASETYPE	DAL_GROUP	/ Data Access Layer base type
	TELESCOP	INTEGRAL	/ Telescope or mission name
	ORIGIN		ISDC		/ Origin of FITS file
	INSTRUME	IBIS		/ Instrument name
	DETNAM		ISGRI		/ Name of the detector layer
	CREATOR		String		/ Program that created this FITS file
	CONFIGUR	String		/ Software configuration
	DATE		UTC_format	/ FITS file creation date
	MJDREF		51544.0		/ Modified Julian Date of time origin
	TIMESYS		TT		/ Time frame system
	TIMEUNIT	d		/ Time unit
	TIMEREF		LOCAL		/ Time reference frame
	TTYPE#		VERSION		/ Version of the instrument characteristic file
	TFORM#		1I		/ Format of column VERSION
	TTYPE#		VSTART		/ Start of validity time in IJD
	TFORM#		1D		/ Format of column VSTART
	TUNIT#		d		/ Unit of column VSTART
	TTYPE#		VSTOP		/ End of validity time in IJD
	TFORM#		1D		/ Format of column VSTOP
	TUNIT#		d		/ Unit of column VSTOP
\END
