import pyfits
import pilton
from ddosa import remove_withtemplate
import os
from numpy import *

icroot="/sps/integral/data/ic/ic_snapshot_20140321"
ibisicroot=icroot+"/ic/ibis/mod"

dc=pilton.heatool("dal_create")
dc["obj_name"]="test_ic/isgri_effc_mod.fits"
dc["template"]="ISGR-EFFC-MOD.tpl"
remove_withtemplate(dc["obj_name"].value+"("+dc["template"].value+")")
dc.run()

f=pyfits.open("test_ic/isgri_effc_mod.fits")
f[1].data=zeros(16+8,dtype=f[1].data.dtype)
f[1].data['PIXEL_GROUPING'][:8]=0
f[1].data['PIXEL_GROUPING'][8:]=16
f[1].data['PIXEL_GROUP'][:8]=arange(8)
f[1].data['PIXEL_GROUP'][8:]=arange(16)
f[1].data['EFFICIENCY']=1

f[1].header['ORIGIN']="ISDC"
f[1].header['VERSION']=1
f[1].header['FILENAME']="isgri_effc_mod_0001.fits"
f[1].header['LOCATN']="isgri_effc_mod_0001.fits"
f[1].header['RESPONSI']="me"
f[1].header['STRT_VAL']="2001-01-01T00:00:00"
f[1].header['END_VAL']="2101-01-01T00:00:00"
f[1].header['VSTART']=0
f[1].header['VSTOP']=300000
f.writeto(ibisicroot+"/isgri_effc_mod_0001.fits",clobber=True)

dc=pilton.heatool("dal_create")
dc["obj_name"]=ibisicroot+"/isgri_effc_mod_idx.fits"
dc["template"]="ISGR-EFFC-MOD-IDX.tpl"
remove_withtemplate(dc["obj_name"].value+"("+dc["template"].value+")")
dc.run()

da=pilton.heatool("dal_attach")
da['Parent']=ibisicroot+"/isgri_effc_mod_idx.fits"
da['Child1']=ibisicroot+"/isgri_effc_mod_0001.fits"
da.run()

f=pyfits.open(da['Parent'].value)
f[1].data[0]['VERSION']=1
f[1].data[0]['VSTART']=0
f[1].data[0]['VSTOP']=30000
f.writeto(da['Parent'].value,clobber=True)

dv=pilton.heatool("dal_verify")
dv["indol"]=ibisicroot+"/isgri_effc_mod_idx.fits"
dv['checksums']="yes"
dv['backpointers']="yes"
dv['detachother']="yes"
dv.run()

da=pilton.heatool("dal_attach")
da['Parent']="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ic_master_file.fits"
da['Child1']=ibisicroot+"/isgri_effc_mod_idx.fits"
da.run()
