import pyfits
f=pyfits.open("/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ic_master_file_osa10.fits")

nhdu=pyfits.BinTableHDU.from_columns(f[3].columns+pyfits.ColDefs([pyfits.Column("ISGR_LUT2_MOD","1I"),pyfits.Column("ISGR_L2RE_MOD","1I"),pyfits.Column("ISGR_MCEC_MOD","1I")]))

for k,v in f[3].header.items():
    if k.startswith("TFORM"): continue
    if k.startswith("TTYPE"): continue
    if k.startswith("TFIELDS"): continue
    if k.startswith("NAXIS"): continue
    print ":",k,"x",v
    nhdu.header[k]=v

nhdu.data[0]['ISGR_LUT2_MOD']=1
nhdu.data[0]['ISGR_L2RE_MOD']=1
nhdu.data[0]['ISGR_MCEC_MOD']=1

f[3]=nhdu
f.writeto("/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ic_master_file.fits",clobber=True)
