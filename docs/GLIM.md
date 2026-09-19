# (Optional) Open3D
```bash
sudo apt update
sudo apt install --no-install-recommends \
    libomp-dev libmetis-dev \
    libfmt-dev libspdlog-dev \
    libboost-serialization-dev \
    libboost-filesystem-dev \
    libboost-thread-dev \
    libboost-program-options-dev \
    libboost-date-time-dev \
    libboost-timer-dev \
    libboost-chrono-dev \
    libboost-regex-dev \
    libboost-random-dev \
    libboost-graph-dev

unset ROS_VERSION
```

```bash
git clone --depth 1 --branch 4.3a1 https://github.com/borglab/gtsam.git
```

Edit
```bash
nano gtsam/gtsam/geometry/SL4.cpp
```
add near the top

```Cpp
#include <cassert>
```

Edit
```bash
nano gtsam/gtsam/slam/FrobeniusFactor.h
```

Around lines 200–225, replace the evaluateError() implementation with this:
```Cpp
Vector evaluateError(const T& T1, const T& T2, OptionalMatrixType H1,
                     OptionalMatrixType H2) const override {
  const bool computeJacobians = H1 || H2;

  // predict T2*T1
  typename T::Jacobian H_T21_T2;
  const T hatT21 =
      traits<T>::Between(T2, T1,
                         computeJacobians ? &H_T21_T2 : nullptr);

  // Calculate \hat T21 * T12_, which is predicted to be I_NxN
  typename T::Jacobian H_pred_hat = T::Jacobian::Zero();
  const T pred =
      traits<T>::Compose(hatT21, T12_,
                         computeJacobians ? &H_pred_hat : nullptr);

  // Move to constructor
  const MatrixN I = MatrixN::Identity();
  const VectorD vecI = Eigen::Map<const VectorD>(I.data());

  // Calculate error
  Eigen::Matrix<double, Dim, T::dimension> H_vec_pred;
  Vector error =
      traits<T>::Vec(pred, computeJacobians ? &H_vec_pred : nullptr) - vecI;

  // Do chain rule only when Jacobians were actually requested
  if (computeJacobians) {
    const auto H_error_hat21 = H_vec_pred * H_pred_hat;
    if (H1) *H1 = H_error_hat21;
    if (H2) *H2 = H_error_hat21 * H_T21_T2;
  }

  return error;
}
```

Build
```bash
cd gtsam
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DGTSAM_BUILD_TESTS=OFF \
      -DGTSAM_BUILD_EXAMPLES_ALWAYS=OFF \
      -DGTSAM_BUILD_TIMING_ALWAYS=OFF \
      -DGTSAM_BUILD_UNSTABLE=ON \
      -DGTSAM_BUILD_PYTHON=OFF \
      -DGTSAM_INSTALL_MATLAB_TOOLBOX=OFF \
      -DGTSAM_WITH_TBB=OFF \
      -DGTSAM_USE_SYSTEM_EIGEN=ON \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DGTSAM_BUILD_WITH_MARCH_NATIVE=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```

gtsam_points
```bash
git clone --depth 1 --branch v1.2.2 https://github.com/koide3/gtsam_points.git
cd gtsam_points
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DBUILD_WITH_CUDA=OFF \
      -DBUILD_WITH_TBB=OFF \
      -DBUILD_WITH_OPENMP=ON \
      -DBUILD_WITH_MARCH_NATIVE=OFF \
      -DBUILD_DEMO=OFF \
      -DBUILD_EXAMPLE=OFF \
      -DBUILD_TOOLS=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_TESTS_PCL=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```


GLIM
```bash
git clone --depth 1 --branch v1.2.2 https://github.com/koide3/glim.git
cd glim
mkdir build
cd build

unset ROS_VERSION
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DBUILD_WITH_CUDA=OFF \
      -DBUILD_WITH_VIEWER=OFF \
      -DBUILD_WITH_OPENCV=OFF \
      -DBUILD_WITH_MARCH_NATIVE=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```
