import pyfits
import pilton
from ddosa import remove_withtemplate
import os
from numpy import *

dc=pilton.heatool("dal_create")
dc["obj_name"]="test_ic/isgr_rise_mod.fits"
dc["template"]="ISGR-RISE-MOD.tpl"
remove_withtemplate(dc["obj_name"].value+"("+dc["template"].value+")")
dc.run()

f=pyfits.open("test_ic/isgri_rise_mod.fits")
d=pyfits.open("/sps/integral/analysis/savchenk/eddosa/0239/lut2_1d_final.fits.FinalizeLUT2.xvbase5.2:0239:cb189c54")[0]

f[1].data=zeros(2048,dtype=f[1].data.dtype)

f[1].data['CHANNEL']=arange(2048)
f[1].data['ENERGY']=d.data[:,30]
f[1].data['CORR']=d.data[:,:]/outer(d.data[:,30],ones(256))

f[1].header['ORIGIN']="ISDC"
f[1].header['VERSION']=1
f[1].header['FILENAME']="isgr_rise_mod_0001.fits"
f[1].header['LOCATN']="isgr_rise_mod_0001.fits"
f[1].header['RESPONSI']="me"
f[1].header['STRT_VAL']="2001-01-01T00:00:00"
f[1].header['END_VAL']="2101-01-01T00:00:00"
f[1].header['VSTART']=0
f[1].header['VSTOP']=300000
f.writeto("/sps/integral/data/ic/ic_snapshot_20140321/ic/ibis/mod/isgr_rise_mod_0001.fits",clobber=True)

dc=pilton.heatool("dal_create")
dc["obj_name"]="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ISGR-RISE-MOD-IDX.fits"
dc["template"]="ISGR-RISE-MOD-IDX.tpl"
remove_withtemplate(dc["obj_name"].value+"("+dc["template"].value+")")
dc.run()

da=pilton.heatool("dal_attach")
da['Parent']="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ISGR-RISE-MOD-IDX.fits"
da['Child1']="/sps/integral/data/ic/ic_snapshot_20140321/ic/ibis/mod/isgr_rise_mod_0001.fits"
da.run()

f=pyfits.open(da['Parent'].value)
f[1].data[0]['VERSION']=1
f[1].data[0]['VSTART']=0
f[1].data[0]['VSTOP']=300000
f.writeto(da['Parent'].value,clobber=True)

dv=pilton.heatool("dal_verify")
dv["indol"]="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ISGR-RISE-MOD-IDX.fits"
dv['checksums']="yes"
dv['backpointers']="yes"
dv['detachother']="yes"
dv.run()

da=pilton.heatool("dal_attach")
da['Parent']="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ic_master_file.fits"
da['Child1']="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ISGR-RISE-MOD-IDX.fits"
da.run()

