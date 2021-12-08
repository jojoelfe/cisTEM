import pycistem
import numpy as np

def test_pixelsize_1_0_kV_300():
    ef = pycistem.ElectronDose()
    ef.Init(300,1.0)
    im = pycistem.Image()
    im.Allocate(1024,1024,1)

    filter_array = ef.CalculateDoseFilterAs1DArray(im,0.0,1.0)

    np.testing.assert_almost_equal(filter_array[0],1.0)
    np.testing.assert_almost_equal(filter_array[10],0.9995435)
    np.testing.assert_almost_equal(filter_array[100],0.98306537)
    np.testing.assert_almost_equal(filter_array[1000],0.9346748)
    np.testing.assert_almost_equal(filter_array[10000],0.9544)
    np.testing.assert_almost_equal(filter_array[100000],0.9333308)

    filter_array = ef.CalculateDoseFilterAs1DArray(im,29.0,30.0)

    np.testing.assert_almost_equal(filter_array[0],1.0)
    np.testing.assert_almost_equal(filter_array[10],0.032447204)
    np.testing.assert_almost_equal(filter_array[100],0.012134116)
    np.testing.assert_almost_equal(filter_array[1000],0.00059190224)
    np.testing.assert_almost_equal(filter_array[10000],0.002078189)
    np.testing.assert_almost_equal(filter_array[100000],0.0005426547)

