/*
 * _EcoTracker.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_EcoTracker.h"

#ifdef USE_OPENTRACKER

namespace kai
{

_EcoTracker::_EcoTracker()
{
}

_EcoTracker::~_EcoTracker()
{
}

bool _EcoTracker::init(void* pKiss)
{
	IF_F(!this->_TrackerBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("useDeepFeature", &m_param.useDeepFeature);
	pK->v("useHogFeature", &m_param.useHogFeature);
	pK->v("useCnFeature", &m_param.useCnFeature);

	pK->v("maxScoreThresh", &m_param.max_score_threshhold);

	pK->v("searchAreaShape", &m_param.search_area_shape);
	pK->v("searchAreaScale", &m_param.search_area_scale);
	pK->v("minImageSampleSize", &m_param.min_image_sample_size);
	pK->v("maxImageSampleSize", &m_param.max_image_sample_size);

	pK->v("refinementIterations", &m_param.refinement_iterations);
	pK->v("newtonIterations", &m_param.newton_iterations);
	pK->v("clampPosition", &m_param.clamp_position);

	pK->v("outputSigmaFactor", &m_param.output_sigma_factor);
	pK->v("learningRate", &m_param.learning_rate);
	pK->v("nSamples", &m_param.nSamples);
	pK->v("sampleReplaceStrategy", &m_param.sample_replace_strategy);
	pK->v("ltSize", &m_param.lt_size);
	pK->v("trainGap", &m_param.train_gap);
	pK->v("skipAfterFrame", &m_param.skip_after_frame);
	pK->v("useDetectionSample", &m_param.use_detection_sample);

	pK->v("useProjectionMatrix", &m_param.use_projection_matrix);
	pK->v("updateProjectionMatrix", &m_param.update_projection_matrix);
	pK->v("projInitMethod", &m_param.proj_init_method);
	pK->v("projectionReg", &m_param.projection_reg);

	pK->v("useSampleMerge", &m_param.use_sample_merge);
	pK->v("sampleMergeType", &m_param.sample_merge_type);
	pK->v("distanceMatrixUpdateType", &m_param.distance_matrix_update_type);

	pK->v("CGiter", &m_param.CG_iter);
	pK->v("initCGiter", &m_param.init_CG_iter);
	pK->v("initGNiter", &m_param.init_GN_iter);
	pK->v("CGuseFR", &m_param.CG_use_FR);
	pK->v("CGstandardAlpha", &m_param.CG_standard_alpha);
	pK->v("CGforgettingRate", &m_param.CG_forgetting_rate);
	pK->v("precondfDataParam", &m_param.precond_data_param);
	pK->v("precondRegParam", &m_param.precond_reg_param);
	pK->v("precondProjParam", &m_param.precond_proj_param);

	pK->v("useRegWindow", &m_param.use_reg_window);
	pK->v("regWindowMin", &m_param.reg_window_min);
	pK->v("regWindowEdge", &m_param.reg_window_edge);
	pK->v("regWindowPower", &m_param.reg_window_power);
	pK->v("regSparsityThreshold", &m_param.reg_sparsity_threshold);

	pK->v("interpolationMethod", &m_param.interpolation_method);
	pK->v("interpolationBicubicA", &m_param.interpolation_bicubic_a);
	pK->v("interpolationCentering", &m_param.interpolation_centering);
	pK->v("interpolationWindowing", &m_param.interpolation_windowing);

	pK->v("nScales", &m_param.number_of_scales);
	pK->v("scaleStep", &m_param.scale_step);
	pK->v("minScaleFactor", &m_param.min_scale_factor);
	pK->v("maxScaleFactor", &m_param.max_scale_factor);

	pK->v("useScaleFilter", &m_param.use_scale_filter);
	pK->v("scaleSigmaFactor", &m_param.scale_sigma_factor);
	pK->v("scaleLearningRate", &m_param.scale_learning_rate);
	pK->v("nScalesFilter", &m_param.number_of_scales_filter);
	pK->v("nInterpScales", &m_param.number_of_interp_scales);
	pK->v("scaleModelFactor", &m_param.scale_model_factor);
	pK->v("scaleStepFilter", &m_param.scale_step_filter);
	pK->v("scaleModelMaxArea", &m_param.scale_model_max_area);
	pK->v("scaleFeature", &m_param.scale_feature);
	pK->v("sNumCompressedDim", &m_param.s_num_compressed_dim);
	pK->v("lambda", &m_param.lambda);
	pK->v("doPolyInterp", &m_param.do_poly_interp);

	pK->v("useGPU", &m_param.use_gpu);
	pK->v("GPUid", &m_param.gpu_id);

	Kiss* pH = pK->o("Hog");
	if(pH)
	{
		pH->v("imgInputW", &m_param.hog_features.img_input_sz.width);
		pH->v("imgInputH", &m_param.hog_features.img_input_sz.height);
		pH->v("imgSampleW", &m_param.hog_features.img_sample_sz.width);
		pH->v("imgSampleH", &m_param.hog_features.img_sample_sz.height);
		pH->v("dataSzBlock0W", &m_param.hog_features.data_sz_block0.width);
		pH->v("dataSzBlock0H", &m_param.hog_features.data_sz_block0.height);

		pH->v("cellSize", &m_param.hog_features.fparams.cell_size);
		pH->v("compressedDim", &m_param.hog_features.fparams.compressed_dim);
		pH->v("nOrients", &m_param.hog_features.fparams.nOrients);
		pH->v("nDim", &m_param.hog_features.fparams.nDim);
		pH->v("penalty", &m_param.hog_features.fparams.penalty);
	}

	return true;
}

bool _EcoTracker::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _EcoTracker::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		track();

		this->autoFPSto();
	}
}

void _EcoTracker::track(void)
{
	NULL_(m_pVision);
	Frame* pFrame = m_pVision->BGR();
	IF_(pFrame->tStamp() <= m_tStampBGR);
	m_tStampBGR = pFrame->tStamp();

	Mat* pMat = pFrame->m();
	IF_(pMat->empty());

	if(m_iSet > m_iInit)
	{
		m_eco.init(*pMat, m_newBB, m_param);
		m_bTracking = true;
		m_rBB = m_newBB;
		m_iInit = m_iSet;
	}
	else
	{
		IF_(!m_bTracking);

		Rect2f r;
		if(!m_eco.update(*pMat, r))
		{
			m_bTracking = false;
			return;
		}

		m_rBB = r;
	}

	vInt4 iBB;
	rect2vInt4(m_rBB,iBB);

	m_bb.x = (double)iBB.x / (double)pMat->cols;
	m_bb.y = (double)iBB.y / (double)pMat->rows;
	m_bb.z = (double)iBB.z / (double)pMat->cols;
	m_bb.w = (double)iBB.w / (double)pMat->rows;
}

}
#endif
