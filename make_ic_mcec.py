import pyfits
import pilton
from ddosa import remove_withtemplate
import os
from numpy import *

icroot="/sps/integral/data/ic/ic_snapshot_20140321"
ibisicroot=icroot+"/ic/ibis/mod"

dc=pilton.heatool("dal_create")
dc["obj_name"]="test_ic/isgr_mcec_mod.fits"
dc["template"]="ISGR-MCEC-MOD.tpl"
remove_withtemplate(dc["obj_name"].value+"("+dc["template"].value+")")
dc.run()

f=pyfits.open("test_ic/isgr_mcec_mod.fits")
f[1].data=zeros(8,dtype=f[1].data.dtype)
f[1].data['MCE']=arange(8)
f[1].data['PHA_OFFSET']=0
f[1].data['PHA_GAIN']=1.
f[1].data['PHA_GAIN2']=0
f[1].data['RT_OFFSET']=0
f[1].data['RT_GAIN']=1

f[1].header['ORIGIN']="ISDC"
f[1].header['VERSION']=1
f[1].header['FILENAME']="isgr_mcec_mod_0001.fits"
f[1].header['LOCATN']="isgr_mcec_mod_0001.fits"
f[1].header['RESPONSI']="me"
f[1].header['STRT_VAL']="2001-01-01T00:00:00"
f[1].header['END_VAL']="2101-01-01T00:00:00"
f[1].header['VSTART']=0
f[1].header['VSTOP']=300000
f.writeto(ibisicroot+"/isgr_mcec_mod_0001.fits",clobber=True)

dc=pilton.heatool("dal_create")
dc["obj_name"]=ibisicroot+"/isgr_mcec_mod_idx.fits"
dc["template"]="ISGR-MCEC-MOD-IDX.tpl"
remove_withtemplate(dc["obj_name"].value+"("+dc["template"].value+")")
dc.run()

da=pilton.heatool("dal_attach")
da['Parent']=ibisicroot+"/isgr_mcec_mod_idx.fits"
da['Child1']=ibisicroot+"/isgr_mcec_mod_0001.fits"
da.run()

f=pyfits.open(da['Parent'].value)
f[1].data[0]['VERSION']=1
f[1].data[0]['VSTART']=0
f[1].data[0]['VSTOP']=30000
f.writeto(da['Parent'].value,clobber=True)

dv=pilton.heatool("dal_verify")
dv["indol"]=ibisicroot+"/isgr_mcec_mod_idx.fits"
dv['checksums']="yes"
dv['backpointers']="yes"
dv['detachother']="yes"
dv.run()

da=pilton.heatool("dal_attach")
da['Parent']="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ic_master_file.fits"
da['Child1']=ibisicroot+"/isgr_mcec_mod_idx.fits"
da.run()

