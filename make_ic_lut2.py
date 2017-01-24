import pyfits
import pilton
from ddosa import remove_withtemplate
import os

f=pyfits.open("test_ic/isgri_lut2_mod.fits")
d=pyfits.open("/sps/integral/analysis/savchenk/eddosa/0239/lut2_1d_final.fits.FinalizeLUT2.xvbase5.2:0239:cb189c54")[0]
f[1].data=d.data[::2,:]

f[1].header['ORIGIN']="ISDC"
f[1].header['VERSION']=1
f[1].header['FILENAME']="isgri_lut2_mod_0001.fits"
f[1].header['LOCATN']="isgri_lut2_mod_0001.fits"
f[1].header['RESPONSI']="me"
f[1].header['STRT_VAL']="2001-01-01T00:00:00"
f[1].header['END_VAL']="2101-01-01T00:00:00"
f[1].header['VSTART']=0
f[1].header['VSTOP']=300000
f.writeto("test_ic/isgri_lut2_mod_0001.fits",clobber=True)

dc=pilton.heatool("dal_create")
dc["obj_name"]="test_ic/isgri_lut2_mod_idx.fits"
dc["template"]="ISGR-LUT2-MOD-IDX.tpl"
remove_withtemplate(dc["obj_name"].value+"("+dc["template"].value+")")
dc.run()

da=pilton.heatool("dal_attach")
da['Parent']="test_ic/isgri_lut2_mod_idx.fits"
da['Child1']="test_ic/isgri_lut2_mod_0001.fits"
da.run()

f=pyfits.open(da['Parent'].value)
f[1].data[0]['VERSION']=1
f[1].data[0]['VSTART']=0
f[1].data[0]['VSTOP']=30000
f.writeto(da['Parent'].value,clobber=True)

dv=pilton.heatool("dal_verify")
dv["indol"]="test_ic/isgri_lut2_mod_idx.fits"
dv['checksums']="yes"
dv['backpointers']="yes"
dv['detachother']="yes"
dv.run()

da=pilton.heatool("dal_attach")
da['Parent']="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ic_master_file.fits"
da['Child1']=os.getcwd()+"/test_ic/isgri_lut2_mod_idx.fits"
da.run()

