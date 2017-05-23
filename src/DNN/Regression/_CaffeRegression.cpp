/*
 * _CaffeRegression.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

#include "_CaffeRegression.h"

#ifdef USE_CAFFE

namespace kai
{
_CaffeRegression::_CaffeRegression()
{
	m_presetDir = "";
	m_fSolverProto = "";
	m_fDeployProto = "";
	m_fPretrainedCaffemodel = "";
	m_fTrainImgList = "";
	m_fTestImgList = "";

	m_width = 224;
	m_height = 224;
	m_nChannel = 3;
	m_targetDim = 6;

	m_layerInTrain = "data";
	m_layerInTest = "data";
	m_layerLabelTrain = "label";
	m_layerLabelTest = "label";

	m_dataSizeTrain = 1000;
	m_dataSizeTest = 1000;

}

_CaffeRegression::~_CaffeRegression()
{
}

bool _CaffeRegression::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,presetDir);
	KISSm(pK,fSolverProto);
	KISSm(pK,fDeployProto);
	KISSm(pK,fPretrainedCaffemodel);
	KISSm(pK,fTrainImgList);
	KISSm(pK,fTestImgList);

	KISSm(pK,width);
	KISSm(pK,height);
	KISSm(pK,nChannel);
	KISSm(pK,targetDim);

	KISSm(pK,layerInTrain);
	KISSm(pK,layerInTest);
	KISSm(pK,layerLabelTrain);
	KISSm(pK,layerLabelTest);

	KISSm(pK,dataSizeTrain);
	KISSm(pK,dataSizeTest);

	return true;
}

void _CaffeRegression::setModeGPU(void)
{
#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
#else
	Caffe::set_mode(Caffe::GPU);
#endif
}

vector<string> _CaffeRegression::split(string str, char c)
{
	vector<string> v;
	string buf = "";
	stringstream ss;

	ss << str;
	while (getline(ss, buf, c))
	{
		v.push_back(buf);
	}

	return v;
}

void _CaffeRegression::readImgListToFloat(string list_path, float *data, float *label, int data_len)
{
	ifstream ifs;
	string str;
	int n = 0;
	ifs.open(list_path.c_str(), std::ios::in);
	if (!ifs)
	{
		LOG(INFO)<< "cannot open " << list_path; return;
	}

//	float mean[m_nChannel] = { 104, 117, 123 };

	while (getline(ifs, str))
	{
		vector<string> entry = split(str, '\t');
		cout << "reading: " << entry[0] << endl;
		cv::Mat img = cv::imread(entry[0]);
		cv::Mat resized_img;
		cv::resize(img, resized_img, cv::Size(m_width, m_height));
		for (int y = 0; y < m_height; y++)
		{
			for (int x = 0; x < m_width; x++)
			{
				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 0
						+ m_width * m_height * m_nChannel * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 0];
//						- mean[0];
				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 1
						+ m_width * m_height * m_nChannel * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 1];
//						- mean[1];
				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 2
						+ m_width * m_height * m_nChannel * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 2];
//						- mean[2];
			}
		}
		for (int i = 0; i < m_targetDim; i++)
		{
			label[n * m_targetDim + i] = stof(entry[i + 1]) / 256.0;
		}
		n++;
	}
}

void _CaffeRegression::readImgFileName(string path, string *infiles)
{
	ifstream ifs;
	ifs.open(path.c_str(), ios::in);
	string str;

	int n = 0;
	while (getline(ifs, str))
	{
		vector<string> entry = split(str, '\t');
		infiles[n] = entry[0];
		n++;
	}

	ifs.close();
}

void _CaffeRegression::run_googlenet_train()
{
	SolverParameter solver_param;
	ReadProtoFromTextFileOrDie(m_fSolverProto.c_str(), &solver_param);
	std::shared_ptr<Solver<float>> solver(SolverRegistry<float>::CreateSolver(solver_param));

	const auto pNet = solver->net();
	const auto pTestNet = solver->test_nets();

	pNet->CopyTrainedLayersFrom(m_fPretrainedCaffemodel.c_str());
	pTestNet[0]->CopyTrainedLayersFrom(m_fPretrainedCaffemodel.c_str());

	float* pDataInTrain = new float[m_dataSizeTrain * m_height * m_width * m_nChannel];
	float* pLabelTrain = new float[m_dataSizeTrain * m_targetDim];
	float* pDataInTest = new float[m_dataSizeTest * m_height * m_width * m_nChannel];
	float* pLabelTest = new float[m_dataSizeTest * m_targetDim];

	readImgListToFloat(m_fTrainImgList.c_str(), pDataInTrain, pLabelTrain, m_dataSizeTrain);
	readImgListToFloat(m_fTestImgList.c_str(), pDataInTest, pLabelTest, m_dataSizeTest);

	const auto pLayerInTrain = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pNet->layer_by_name(m_layerInTrain.c_str()));
	const auto pLayerInTest = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pTestNet[0]->layer_by_name(m_layerInTest.c_str()));

	float* pDummyDataTrain = new float[m_dataSizeTrain];
	float* pDummtDataTest = new float[m_dataSizeTest];
	for (int i = 0; i < m_dataSizeTrain; i++)
		pDummyDataTrain[i] = 0;
	for (int i = 0; i < m_dataSizeTest; i++)
		pDummtDataTest[i] = 0;

	pLayerInTrain->Reset((float*) pDataInTrain, (float*) pDummyDataTrain, m_dataSizeTrain);
	pLayerInTest->Reset((float*) pDataInTest, (float*) pDummtDataTest, m_dataSizeTest);

	const auto pLayerLabelTrain = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pNet->layer_by_name(m_layerLabelTrain.c_str()));
	const auto pLayerLabelTest = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(pTestNet[0]->layer_by_name(m_layerLabelTest.c_str()));

	pLayerLabelTrain->Reset((float*) pLabelTrain, (float*) pDummyDataTrain, m_dataSizeTrain);
	pLayerLabelTest->Reset((float*) pLabelTest, (float*) pDummtDataTest, m_dataSizeTest);

	LOG_I("Solve start");
	solver->Solve();

	delete[] pDataInTrain;
	delete[] pLabelTrain;
	delete[] pDummyDataTrain;
	delete[] pDataInTest;
	delete[] pLabelTest;
	delete[] pDummtDataTest;
}

void _CaffeRegression::run_googlenet_test()
{
	int batch_size = 10;
	int test_data_size = 1000;
	int batch_iter = test_data_size / batch_size;
	int test_input_num = 2;

	string *infiles = new string[test_data_size];
	ofstream ofs;
	ofs.open("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\result.txt", ios::out);
	readImgFileName("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\test.txt", infiles);

	float *test_input_data;
	float *test_label;
	test_input_data = new float[test_data_size * m_height * m_width * m_nChannel];
	test_label = new float[test_data_size * m_targetDim];

	Net<float> test_net("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\deploy.prototxt", TEST);
	test_net.CopyTrainedLayersFrom("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\snapshot\\bvlc_googlenet_iter_80000.caffemodel");

	readImgListToFloat("G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\test.txt", test_input_data, test_label, test_data_size);

	const auto input_test_layer = boost::dynamic_pointer_cast<MemoryDataLayer<float>>(test_net.layer_by_name("data"));
	float *test_dummy = new float[test_data_size];
	for (int i = 0; i < test_data_size; i++)
		test_dummy[i] = 0.0f;

	cv::Mat oimg;
	cv::Mat resized_oimg;

	for (int batch = 0; batch < batch_iter; batch++)
	{
		input_test_layer->Reset(
				(float*) test_input_data
						+ batch * m_width * m_height * m_nChannel * batch_size,
				test_dummy + batch * batch_size, batch_size);
		const auto result = test_net.Forward();

		const auto data = result[1]->cpu_data();
		for (int i = 0; i < batch_size; i++)
		{
			int total_id = batch * batch_size + i;
			oimg = cv::imread(infiles[total_id]);
			resized_oimg;
			cv::resize(oimg, resized_oimg, cv::Size(m_width, m_height));

			int *val = new int(m_targetDim);
			for (int j = 0; j < m_targetDim; j++)
			{
				val[j] = (int) (data[i * m_targetDim + j] * 256);
				if (val[j] < 0)
					val[j] = 0;
				if (j == 0 && val[j] > m_width)
					val[j] = m_width - 1;
				if (j == 1 && val[j] > m_height)
					val[j] = m_height - 1;
				if (j >= 3 && val[j] > 255)
					val[j] = 255;
			}
			cv::circle(resized_oimg, cv::Point(val[0], val[1]), val[2],
					cv::Scalar(val[3], val[4], val[5]), 3);

			stringstream ofname;
			ofname
					<< "G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\output_img\\"
					<< setw(5) << setfill('0') << i2str(total_id) << ".png";
			cv::imwrite(ofname.str(), resized_oimg);
		}
	}

	ofs.close();
	oimg.release();
	resized_oimg.release();
	delete[] test_input_data;
	test_input_data = 0;
	delete[] test_label;
	test_label = 0;
	delete[] test_dummy;
	test_dummy = 0;
	delete[] infiles;
	infiles = 0;
}

}

#endif
