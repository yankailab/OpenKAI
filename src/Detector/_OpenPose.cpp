/*
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */
#include "_OpenPose.h"

namespace kai
{

_OpenPose::_OpenPose()
{
	m_nW = 368;
	m_nH = 368;
	m_bSwapRB = false;
	m_vMean.init();
	m_scale = 1.0 / 255.0;

	m_iBackend = dnn::DNN_BACKEND_OPENCV;
	m_iTarget = dnn::DNN_TARGET_CPU;
}

_OpenPose::~_OpenPose()
{
}

bool _OpenPose::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, nW);
	KISSm(pK, nH);
	KISSm(pK, iBackend);
	KISSm(pK, iTarget);
	KISSm(pK, bSwapRB);
	KISSm(pK, scale);
	pK->v("meanB", &m_vMean.x);
	pK->v("meanG", &m_vMean.y);
	pK->v("meanR", &m_vMean.z);

	m_net = readNetFromCaffe(m_modelFile, m_trainedFile);
	IF_Fl(m_net.empty(), "read Net failed");

	m_net.setPreferableBackend(m_iBackend);
	m_net.setPreferableTarget(m_iTarget);

	return true;
}

bool _OpenPose::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _OpenPose::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		updateObj();

		if (m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _OpenPose::check(void)
{
	NULL__(m_pVision, -1);
	Frame* pBGR = m_pVision->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_fBGR.tStamp(), -1);

	return 0;
}

bool _OpenPose::detect(void)
{
	IF_F(check() < 0);

	Frame* pBGR = m_pVision->BGR();
	m_fBGR.copy(*pBGR);
	Mat mIn = *m_fBGR.m();

	m_blob = blobFromImage(mIn, m_scale,
			Size((int) ((368 * mIn.cols) / mIn.rows), 368),
			Scalar(m_vMean.x, m_vMean.y, m_vMean.z), m_bSwapRB, false);
	m_net.setInput(m_blob);

	cv::Mat netOutputBlob = m_net.forward();

	splitNetOutputBlobToParts(netOutputBlob, cv::Size(mIn.cols, mIn.rows), m_netOutputParts);

	int keyPointId = 0;
	for (int i = 0; i < OP_N_POINTS; ++i)
	{
		std::vector<KeyPoint> keyPoints;
		getKeyPoints(m_netOutputParts[i], 0.1, keyPoints);

		std::cout << "Keypoints - " << keypointsMapping[i] << " : " << keyPoints << std::endl;

		for (int i = 0; i < keyPoints.size(); ++i, ++keyPointId)
		{
			keyPoints[i].id = keyPointId;
		}

		m_detectedKeypoints.push_back(keyPoints);
		m_keyPointsList.insert(m_keyPointsList.end(), keyPoints.begin(), keyPoints.end());
	}


	return true;
}

void _OpenPose::getKeyPoints(Mat& probMap, double threshold,
		vector<KeyPoint>& keyPoints)
{
	Mat smoothProbMap;
	GaussianBlur(probMap, smoothProbMap, Size(3, 3), 0, 0);

	Mat maskedProbMap;
	cv::threshold(smoothProbMap, maskedProbMap, threshold, 255, THRESH_BINARY);

	maskedProbMap.convertTo(maskedProbMap, CV_8U, 1);

	vector<vector<Point> > contours;
	findContours(maskedProbMap, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

	for (int i = 0; i < contours.size(); ++i)
	{
		Mat blobMask = Mat::zeros(smoothProbMap.rows, smoothProbMap.cols,
				smoothProbMap.type());

		fillConvexPoly(blobMask, contours[i], Scalar(1));

		double maxVal;
		Point maxLoc;

		minMaxLoc(smoothProbMap.mul(blobMask), 0, &maxVal, 0, &maxLoc);

		keyPoints.push_back(
				KeyPoint(maxLoc, probMap.at<float>(maxLoc.y, maxLoc.x)));
	}
}

void _OpenPose::populateColorPalette(vector<Scalar>& colors, int nColors)
{
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dis1(64, 200);
	uniform_int_distribution<> dis2(100, 255);
	uniform_int_distribution<> dis3(100, 255);

	for (int i = 0; i < nColors; ++i)
	{
		colors.push_back(Scalar(dis1(gen), dis2(gen), dis3(gen)));
	}
}

void _OpenPose::splitNetOutputBlobToParts(Mat& netOutputBlob,
		const Size& targetSize, vector<Mat>& netOutputParts)
{
	int nParts = netOutputBlob.size[1];
	int h = netOutputBlob.size[2];
	int w = netOutputBlob.size[3];

	for (int i = 0; i < nParts; ++i)
	{
		Mat part(h, w, CV_32F, netOutputBlob.ptr(0, i));

		Mat resizedPart;

		resize(part, resizedPart, targetSize);

		netOutputParts.push_back(resizedPart);
	}
}

void _OpenPose::populateInterpPoints(const Point& a, const Point& b,
		int numPoints, vector<Point>& interpCoords)
{
	float xStep = ((float) (b.x - a.x)) / (float) (numPoints - 1);
	float yStep = ((float) (b.y - a.y)) / (float) (numPoints - 1);

	interpCoords.push_back(a);

	for (int i = 1; i < numPoints - 1; ++i)
	{
		interpCoords.push_back(Point(a.x + xStep * i, a.y + yStep * i));
	}

	interpCoords.push_back(b);
}

void _OpenPose::getValidPairs(const vector<Mat>& netOutputParts,
		const vector<vector<KeyPoint>>& detectedKeypoints,
		vector<vector<ValidPair>>& validPairs, set<int>& invalidPairs)
{
	int nInterpSamples = 10;
	float pafScoreTh = 0.1;
	float confTh = 0.7;

	for (int k = 0; k < mapIdx.size(); ++k)
	{

		//A->B constitute a limb
		Mat pafA = netOutputParts[mapIdx[k].first];
		Mat pafB = netOutputParts[mapIdx[k].second];

		//Find the keypoints for the first and second limb
		const vector<KeyPoint>& candA = detectedKeypoints[posePairs[k].first];
		const vector<KeyPoint>& candB = detectedKeypoints[posePairs[k].second];

		int nA = candA.size();
		int nB = candB.size();

		/*
		 # If keypoints for the joint-pair is detected
		 # check every joint in candA with every joint in candB
		 # Calculate the distance vector between the two joints
		 # Find the PAF values at a set of interpolated points between the joints
		 # Use the above formula to compute a score to mark the connection valid
		 */

		if (nA != 0 && nB != 0)
		{
			vector<ValidPair> localValidPairs;

			for (int i = 0; i < nA; ++i)
			{
				int maxJ = -1;
				float maxScore = -1;
				bool found = false;

				for (int j = 0; j < nB; ++j)
				{
					pair<float, float> distance(
							candB[j].point.x - candA[i].point.x,
							candB[j].point.y - candA[i].point.y);

					float norm = sqrt(
							distance.first * distance.first
									+ distance.second * distance.second);

					if (!norm)
					{
						continue;
					}

					distance.first /= norm;
					distance.second /= norm;

					//Find p(u)
					vector<Point> interpCoords;
					populateInterpPoints(candA[i].point, candB[j].point,
							nInterpSamples, interpCoords);
					//Find L(p(u))
					vector<pair<float, float>> pafInterp;
					for (int l = 0; l < interpCoords.size(); ++l)
					{
						pafInterp.push_back(
								pair<float, float>(
										pafA.at<float>(interpCoords[l].y,
												interpCoords[l].x),
										pafB.at<float>(interpCoords[l].y,
												interpCoords[l].x)));
					}

					vector<float> pafScores;
					float sumOfPafScores = 0;
					int numOverTh = 0;
					for (int l = 0; l < pafInterp.size(); ++l)
					{
						float score = pafInterp[l].first * distance.first
								+ pafInterp[l].second * distance.second;
						sumOfPafScores += score;
						if (score > pafScoreTh)
						{
							++numOverTh;
						}

						pafScores.push_back(score);
					}

					float avgPafScore = sumOfPafScores
							/ ((float) pafInterp.size());

					if (((float) numOverTh) / ((float) nInterpSamples) > confTh)
					{
						if (avgPafScore > maxScore)
						{
							maxJ = j;
							maxScore = avgPafScore;
							found = true;
						}
					}

				}/* j */

				if (found)
				{
					localValidPairs.push_back(
							ValidPair(candA[i].id, candB[maxJ].id, maxScore));
				}

			}/* i */

			validPairs.push_back(localValidPairs);

		}
		else
		{
			invalidPairs.insert(k);
			validPairs.push_back(vector<ValidPair>());
		}
	}/* k */
}

void _OpenPose::getPersonwiseKeypoints(
		const vector<vector<ValidPair>>& validPairs,
		const set<int>& invalidPairs, vector<vector<int>>& personwiseKeypoints)
{
	for (int k = 0; k < mapIdx.size(); ++k)
	{
		if (invalidPairs.find(k) != invalidPairs.end())
		{
			continue;
		}

		const vector<ValidPair>& localValidPairs(validPairs[k]);

		int indexA(posePairs[k].first);
		int indexB(posePairs[k].second);

		for (int i = 0; i < localValidPairs.size(); ++i)
		{
			bool found = false;
			int personIdx = -1;

			for (int j = 0; !found && j < personwiseKeypoints.size(); ++j)
			{
				if (indexA < personwiseKeypoints[j].size()
						&& personwiseKeypoints[j][indexA]
								== localValidPairs[i].aId)
				{
					personIdx = j;
					found = true;
				}
			}/* j */

			if (found)
			{
				personwiseKeypoints[personIdx].at(indexB) =
						localValidPairs[i].bId;
			}
			else if (k < 17)
			{
				vector<int> lpkp(vector<int>(18, -1));

				lpkp.at(indexA) = localValidPairs[i].aId;
				lpkp.at(indexB) = localValidPairs[i].bId;

				personwiseKeypoints.push_back(lpkp);
			}

		}/* i */
	}/* k */
}

bool _OpenPose::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	std::vector<cv::Scalar> colors;
	populateColorPalette(colors, OP_N_POINTS);

	for (int i = 0; i < OP_N_POINTS; ++i)
	{
		for (int j = 0; j < m_detectedKeypoints[i].size(); ++j)
		{
			cv::circle(*pMat, m_detectedKeypoints[i][j].point, 5, colors[i], -1, cv::LINE_AA);
		}
	}

	std::vector<std::vector<ValidPair>> validPairs;
	std::set<int> invalidPairs;
	getValidPairs(m_netOutputParts, m_detectedKeypoints, validPairs, invalidPairs);

	std::vector<std::vector<int>> personwiseKeypoints;
	getPersonwiseKeypoints(validPairs, invalidPairs, personwiseKeypoints);

	for (int i = 0; i < OP_N_POINTS - 1; ++i)
	{
		for (int n = 0; n < personwiseKeypoints.size(); ++n)
		{
			const std::pair<int, int>& posePair = posePairs[i];
			int indexA = personwiseKeypoints[n][posePair.first];
			int indexB = personwiseKeypoints[n][posePair.second];

			if (indexA == -1 || indexB == -1)
			{
				continue;
			}

			const KeyPoint& kpA = m_keyPointsList[indexA];
			const KeyPoint& kpB = m_keyPointsList[indexB];

			cv::line(*pMat, kpA.point, kpB.point, colors[i], 3, cv::LINE_AA);
		}
	}

	return true;
}

}
