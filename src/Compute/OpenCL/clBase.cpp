#include "clBase.h"

#ifdef USE_OPENCL

namespace kai
{

    clBase::clBase()
    {
    }

    clBase::~clBase()
    {
    }

    bool clBase::init(void *pKiss)
    {
        IF_F(!this->BASE::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

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

        return true;
    }

    void clBase::update(void)
    {
    }

}
#endif
