#include "clBase.h"

namespace kai
{

    clBase::clBase()
    {
        m_iTargetPlatformIdx = 0;
        m_iTargetDevIdx = 0;
        m_targetPlatformName = "";
        m_targetDevName = "";

        m_pCLcontextProps = NULL;
        m_pCLqProp = NULL;

        m_fKernel = "";
        m_buildOpt = "";
    }

    clBase::~clBase()
    {
        clReleaseKernel(m_clKernel);
        clReleaseCommandQueue(m_clCmdQ);
        clReleaseProgram(m_clProgram);
        clReleaseContext(m_clContext);
    }

    bool clBase::init(void *pKiss)
    {
        IF_F(!this->BASE::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("fKernel", &m_fKernel);
        pK->v("buildOpt", &m_buildOpt);

        IF_F(!setupCL());
        IF_F(!setupKernel());

        return true;
    }

	bool clBase::link(void)
    {
		IF_F(!this->BASE::link());
		Kiss *pK = (Kiss *)m_pKiss;

        // string n;
        // n = "";
        // pK->v("", &n);
        // m_p = ( *)(pK->findModule(n));
        // IF_Fl(!m_p, n + ": not found");

		return true;
    }

	int clBase::check(void)
    {

    }

    bool clBase::setupCL(void)
    {
        cl_platform_id platform;
        cl_int r;

        r = clGetPlatformIDs(1, &platform, NULL);
        IF_F(r != CL_SUCCESS);

        r = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &m_clDev, nullptr);
        IF_F(r != CL_SUCCESS);

        m_clContext = clCreateContext(m_pCLcontextProps, 1, &m_clDev, NULL, NULL, &r);
        IF_F(r != CL_SUCCESS);

        m_clCmdQ = clCreateCommandQueueWithProperties(m_clContext, m_clDev, m_pCLqProp, &r);
        IF_F(r != CL_SUCCESS);

        return true;
    }

    bool clBase::setupKernel(void)
    {
        FILE *fp = fopen(m_fKernel.c_str(), "r");
        IF_F(!fp);

        cl_int r;

        char* pSrc = (char*)malloc(CLBASE_SRC_NB);
        size_t nSrc = fread(pSrc, 1, CLBASE_SRC_NB, fp);
        fclose(fp);

        m_clProgram = clCreateProgramWithSource(m_clContext, 1, (const char **)&pSrc, (const size_t *)&nSrc, &r);
        free(pSrc);
        IF_F(r != CL_SUCCESS);

        r = clBuildProgram(m_clProgram, 1, &m_clDev, m_buildOpt.c_str(), NULL, NULL);
        if (r == CL_SUCCESS)
        {
            m_clKernel = clCreateKernel(m_clProgram, m_kName.c_str(), &r);
            IF_F(r != CL_SUCCESS);

            return true;
        }

        LOG_E("clBuildProgram: " + i2str(r));

        size_t nLog;
        r = clGetProgramBuildInfo(m_clProgram, m_clDev, CL_PROGRAM_BUILD_LOG, 0, NULL, &nLog);
        IF_F(r != CL_SUCCESS);

        char *pLog = (char *)malloc(nLog);
        NULL_F(pLog);

        r = clGetProgramBuildInfo(m_clProgram, m_clDev, CL_PROGRAM_BUILD_LOG, nLog, pLog, NULL);
        IF_F(r != CL_SUCCESS);

        fprintf(stderr, "clProgramBuild:\n%s\n", pLog);
        free(pLog);

        return false;
    }

    void clBase::getPlatformInfo(void)
    {
        cl_uint platformNumber = 0;
        cl_platform_id platformIds[8];
        clGetPlatformIDs(8, platformIds, &platformNumber);

        char str[256];

        char string[256];
        cl_device_type type;
        cl_uint value;
        size_t sizes[3];
        cl_ulong ulvalue;
        for (int i = 0; i < platformNumber; i++)
        {
            sprintf(str, "platform idx : %d\n", i);
            LOG_I(str);

            cl_platform_id platform = platformIds[i];
            clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, 256, string, NULL);
            sprintf(str, "platform vendor : %s\n", string);
            LOG_I(str);

            clGetPlatformInfo(platform, CL_PLATFORM_NAME, 256, string, NULL);
            sprintf(str, "platform name : %s\n", string);
            LOG_I(str);

            clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, string, NULL);
            sprintf(str, "platform version : %s\n", string);
            LOG_I(str);

            cl_uint deviceNumber = 0;
            cl_device_id deviceIds[8];
            clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 8, deviceIds, &deviceNumber);
            for (int j = 0; j < deviceNumber; j++)
            {
                sprintf(str, "	device idx : %d\n", j);
                LOG_I(str);

                cl_device_id device = deviceIds[j];
                clGetDeviceInfo(device, CL_DEVICE_NAME, 256, string, NULL);
                sprintf(str, "	device name : %s\n", string);
                LOG_I(str);

                clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
                if (type == CL_DEVICE_TYPE_CPU)
                {
                    sprintf(str, "	device type : CPU\n");
                    LOG_I(str);
                }

                if (type == CL_DEVICE_TYPE_GPU)
                {
                    sprintf(str, "	device type : GPU\n");
                    LOG_I(str);
                }

                if (type == CL_DEVICE_TYPE_ACCELERATOR)
                {
                    sprintf(str, "	device type : ACCELERATOR\n");
                    LOG_I(str);
                }

                clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &value, NULL);
                sprintf(str, "	device max compute units : %d\n", value);
                LOG_I(str);

                clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 3, sizes, NULL);
                sprintf(str, "	device max work item sizes : [%d][%d][%d]\n", sizes[0], sizes[1], sizes[2]);
                LOG_I(str);

                clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_uint), &value, NULL);
                sprintf(str, "	device max work group size : %d\n", value);
                LOG_I(str);

                clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulvalue, NULL);
                sprintf(str, "	device max mem alloc size : %d\n", ulvalue);
                LOG_I(str);

                clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &ulvalue, NULL);
                sprintf(str, "	device max constant buffer size : %d\n", ulvalue);
                LOG_I(str);
            }
        }
    }

}
