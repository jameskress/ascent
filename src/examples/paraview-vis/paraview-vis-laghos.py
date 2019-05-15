import sys
# CHANGE this path to the result of:
# $(spack location --install-dir paraview)
paraview_path = "/home/danlipsa/projects/ascent/build/spack/opt/spack/linux-ubuntu18.04-x86_64/gcc-7.3.0/paraview-master-dn6y4ofccm4eg3wya6u5uvgz6x6aar5w"
paraview_path = paraview_path + "/lib/python2.7/site-packages"
sys.path.append(paraview_path)


# Same Python interpreter for all time steps
# We use count for one time initializations
try:
    count = count + 1
except NameError:
    count = 0

# ParaView API
# WARNING: this does not work inside the plugin
#          unless you have the same import in paraview-vis.py
from mpi4py import MPI
import paraview
paraview.options.batch = True
paraview.options.symmetric = True
from paraview.simple import LoadPlugin, Show, ColorBy,\
    GetColorTransferFunction, GetActiveView, ResetCamera, GetActiveCamera,\
    GetScalarBar, Render, WriteImage, CreateWriter
import ascent_extract

node = ascent_extract.ascent_data().child(0)
domain_id = node["state/domain_id"]
cycle = node["state/cycle"]
imageName = "image_{0:04d}.png".format(int(cycle))
dataName = "paraviewdata_{0:04d}".format(int(cycle))
scriptName = "/home/danlipsa/projects/ascent/src/examples/paraview-vis/paraview_ascent_source.py"
LoadPlugin(scriptName, remote=True, ns=globals())
ascentSource = AscentSource()
# show the boundary topology. For the main topology use Port 0.
ascentSource.Port = 1
rep = Show()

ColorBy(rep, ("CELLS", "boundary_attribute"))
# rescale transfer function
lut = GetColorTransferFunction('boundary_attribute')
lut.RescaleTransferFunction(1, 3)
# show color bar
renderView1 = GetActiveView()
lut = GetScalarBar(lut, renderView1)
lut.Title = 'boundary_attribute'
lut.ComponentTitle = ''
# set color bar visibility
lut.Visibility = 1
# show color legend
rep.SetScalarBarVisibility(renderView1, True)
ResetCamera()
if count == 0:
    cam = GetActiveCamera()
    cam.Elevation(30)
    cam.Azimuth(30)
Render()
WriteImage(imageName)
writer = CreateWriter(dataName + ".pvtu", ascentSource)
writer.UpdatePipeline()


# # VTK API
# from ascent_to_vtk import AscentSource, write_data
# ascentSource = AscentSource()
# ascentSource.Update()
# write_data("vtkdata-main", ascentSource.GetNode(),
#            ascentSource.GetOutputDataObject(0))


# # Python API
# from ascent_to_vtk import ascent_to_vtk, write_data, write_json
# node = ascent_data().child(0)
# write_json(node)
# data = ascent_to_vtk(node, "main")
# write_data("pythondata-main", node, data)
# data = ascent_to_vtk(node, "boundary")
# write_data("pythondata-boundary", node, data)
