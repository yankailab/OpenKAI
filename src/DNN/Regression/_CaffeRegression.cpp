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
}

_CaffeRegression::~_CaffeRegression()
{
}

bool _CaffeRegression::init(JSON* pJson)
{
	//Setup Caffe Classifier
	string caffeDir = "";
	string modelFile;
	string trainedFile;
	string meanFile;
	string labelFile;
	string presetDir = "";

	setup(caffeDir + modelFile, caffeDir + trainedFile, caffeDir + meanFile,
			caffeDir + labelFile, 1);
	LOG(INFO)<<"Caffe Initialized";

	double FPS = DEFAULT_FPS;
	this->setTargetFPS(FPS);

//	m_pFrame = new Frame();

	return true;
}

void _CaffeRegression::setup(const string& model_file,
		const string& trained_file, const string& mean_file,
		const string& label_file, int batch_size)
{

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
		LOG(INFO)<< "cannot open " << list_path; return;}

	float mean[CHANNEL] =
	{ 104, 117, 123 };

	while (getline(ifs, str))
	{
		vector<string> entry = split(str, '\t');
		cout << "reading: " << entry[0] << endl;
		cv::Mat img = cv::imread(entry[0]);
		cv::Mat resized_img;
		cv::resize(img, resized_img, cv::Size(WIDTH, HEIGHT));
		for (int y = 0; y < HEIGHT; y++)
		{
			for (int x = 0; x < WIDTH; x++)
			{
				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 0
						+ WIDTH * HEIGHT * CHANNEL * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 0]
						- mean[0];
				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 1
						+ WIDTH * HEIGHT * CHANNEL * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 1]
						- mean[1];
				data[y * resized_img.cols + x
						+ resized_img.cols * resized_img.rows * 2
						+ WIDTH * HEIGHT * CHANNEL * n] = resized_img.data[y
						* resized_img.step + x * resized_img.elemSize() + 2]
						- mean[2];
			}
		}
		for (int i = 0; i < TARGET_DIM; i++)
		{
			label[n * TARGET_DIM + i] = stof(entry[i + 1]) / 256.0;
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
	ReadProtoFromTextFileOrDie(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\solver.prototxt",
			&solver_param);
	shared_ptr<Solver<float>> solver(SolverRegistry<float>::CreateSolver(solver_param));
	const auto net = solver->net();
	const auto test_net = solver->test_nets();

	net->CopyTrainedLayersFrom(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\bvlc_googlenet.caffemodel");
	test_net[0]->CopyTrainedLayersFrom(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\bvlc_googlenet.caffemodel");

	int train_data_size = 1000;
	float *train_input_data;
	float *train_label;

	int test_data_size = 1000;
	float *test_input_data;
	float *test_label;

	train_input_data = new float[train_data_size * HEIGHT * WIDTH * CHANNEL];
	train_label = new float[train_data_size * TARGET_DIM];
	test_input_data = new float[test_data_size * HEIGHT * WIDTH * CHANNEL];
	test_label = new float[test_data_size * TARGET_DIM];

	readImgListToFloat(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\train.txt",
			train_input_data, train_label, train_data_size);
	readImgListToFloat(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\test.txt",
			test_input_data, test_label, test_data_size);

	const auto train_input_layer = boost::dynamic_pointer_cast<
			MemoryDataLayer<float>>(net->layer_by_name("data"));
	const auto test_input_layer = boost::dynamic_pointer_cast<
			MemoryDataLayer<float>>(test_net[0]->layer_by_name("data"));
	float *train_dummy = new float[train_data_size];
	float *test_dummy = new float[test_data_size];
	for (int i = 0; i < train_data_size; i++)
		train_dummy[i] = 0;
	for (int i = 0; i < test_data_size; i++)
		test_dummy[i] = 0;
	train_input_layer->Reset((float*) train_input_data, (float*) train_dummy,
			train_data_size);
	test_input_layer->Reset((float*) test_input_data, (float*) test_dummy,
			test_data_size);

	const auto train_label_layer = boost::dynamic_pointer_cast<
			MemoryDataLayer<float>>(net->layer_by_name("label"));
	const auto test_label_layer = boost::dynamic_pointer_cast<
			MemoryDataLayer<float>>(test_net[0]->layer_by_name("label"));
	train_label_layer->Reset((float*) train_label, (float*) train_dummy,
			train_data_size);
	test_label_layer->Reset((float*) test_label, (float*) test_dummy,
			test_data_size);

	LOG(INFO)<< "Solve start.";
	solver->Solve();

	delete[] train_input_data;
	train_input_data = 0;
	delete[] train_label;
	train_label = 0;
	delete[] train_dummy;
	train_dummy = 0;
	delete[] test_input_data;
	test_input_data = 0;
	delete[] test_label;
	test_label = 0;
	delete[] test_dummy;
	test_dummy = 0;
}

void _CaffeRegression::run_googlenet_test()
{
	int batch_size = 10;
	int test_data_size = 1000;
	int batch_iter = test_data_size / batch_size;
	int test_input_num = 2;

	string *infiles = new string[test_data_size];
	ofstream ofs;
	ofs.open(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\result.txt",
			ios::out);
	readImgFileName(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\test.txt",
			infiles);

	float *test_input_data;
	float *test_label;
	test_input_data = new float[test_data_size * HEIGHT * WIDTH * CHANNEL];
	test_label = new float[test_data_size * TARGET_DIM];

	Net<float> test_net(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\deploy.prototxt",
			TEST);
	test_net.CopyTrainedLayersFrom(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\snapshot\\bvlc_googlenet_iter_80000.caffemodel");

	readImgListToFloat(
			"G:\\Projects\\roundRegression\\caffe-master\\roundRegression\\test.txt",
			test_input_data, test_label, test_data_size);

	const auto input_test_layer = boost::dynamic_pointer_cast<
			MemoryDataLayer<float>>(test_net.layer_by_name("data"));
	float *test_dummy = new float[test_data_size];
	for (int i = 0; i < test_data_size; i++)
		test_dummy[i] = 0.0f;
	cv::Mat oimg;
	cv::Mat resized_oimg;

	for (int batch = 0; batch < batch_iter; batch++)
	{
		input_test_layer->Reset(
				(float*) test_input_data
						+ batch * WIDTH * HEIGHT * CHANNEL * batch_size,
				test_dummy + batch * batch_size, batch_size);
		const auto result = test_net.Forward();

		const auto data = result[1]->cpu_data();
		for (int i = 0; i < batch_size; i++)
		{
			int total_id = batch * batch_size + i;
			oimg = cv::imread(infiles[total_id]);
			resized_oimg;
			cv::resize(oimg, resized_oimg, cv::Size(WIDTH, HEIGHT));

			int *val = new int(TARGET_DIM);
			for (int j = 0; j < TARGET_DIM; j++)
			{
				val[j] = (int) (data[i * TARGET_DIM + j] * 256);
				if (val[j] < 0)
					val[j] = 0;
				if (j == 0 && val[j] > WIDTH)
					val[j] = WIDTH - 1;
				if (j == 1 && val[j] > HEIGHT)
					val[j] = HEIGHT - 1;
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
