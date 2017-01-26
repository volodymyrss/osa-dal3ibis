import pyfits
import pilton
from ddosa import remove_withtemplate
import os
from numpy import *

infn="/sps/integral/data/reduced/ddcache//byrev/0239/ISGRI_RISE_MOD.v0//2e84304c/isgr_rise_mod_0239.fits.gz"

f=pyfits.open(infn)

outfn="/sps/integral/data/ic/ic_snapshot_20140321/ic/ibis/mod/"+f[1].header['FILENAME']

f.writeto(outfn,clobber=True)

listfn="isgr_rise_mod_list.txt"

open(listfn,"w").write(outfn+"\n")

da=pilton.heatool("txt2idx")
da['index']="/sps/integral/data/ic/ic_snapshot_20140321/idx/ic/ISGR-RISE-MOD-IDX.fits"
da['template']="ISGR-RISE-MOD-IDX.tpl"
da['update']=1
da['element']=listfn
da.run()

f=pyfits.open(da['index'].value)
f[1].header['CREATOR']=""
f[1].header['CONFIGUR']=""
f.writeto(da['index'].value,clobber=True)

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

