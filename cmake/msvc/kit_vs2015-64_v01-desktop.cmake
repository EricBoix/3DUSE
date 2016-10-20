# MSVC - KIT VS 2015 x64 - V01

if( BUILD_GUI_QT5 )
  set(QT5_DIR           ${VCITY_KIT_ROOT}/Qt/Qt5.6.0/5.6/msvc2015_64)
endif()

###### PCL and its sub-dependencies
set(BOOST_ROOT          ${VCITY_KIT_ROOT}/boost_1_59_0)
set(FLANN_ROOT          ${VCITY_KIT_ROOT}/PCL/flann-1.8.4)
