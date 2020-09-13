#include <iostream>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/BucketLocationConstraint.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/core/utils/UUID.h>
#include <aws/core/utils/StringUtils.h>

#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>

#include <tiffio.h>
#include <tiffio.hxx>

bool CreateTIFF(const Aws::String& bucketName, 
    const Aws::S3::Model::BucketLocationConstraint& region)
{
    Aws::Client::ClientConfiguration config;
    config.endpointOverride = "https://dtn01.sdcc.bnl.gov:8000";
    config.verifySSL = false;

    Aws::S3::S3Client s3_client(config);

    // Aws::S3::Model::CreateBucketRequest request;
    // request.SetBucket(bucketName);

    // You only need to set the AWS Region for the bucket if it is 
    // other than US East (N. Virginia) us-east-1.
    // if (region != Aws::S3::Model::BucketLocationConstraint::us_east_1)
    // {
    //     Aws::S3::Model::CreateBucketConfiguration bucket_config;
    //     bucket_config.SetLocationConstraint(region);

    //     request.SetCreateBucketConfiguration(bucket_config);
    // }

    // Aws::S3::Model::CreateBucketOutcome outcome = 
    //     s3_client.CreateBucket(request);

    // if (!outcome.IsSuccess())
    // {
    //     auto err = outcome.GetError();
    //     std::cout << "Error: CreateBucket: " <<
    //         err.GetExceptionName() << ": " << err.GetMessage() << std::endl;

    //     return false;
    // }

    // std::cout << "Created bucket " << bucketName << " in the specified AWS Region." << std::endl;

    // Now write a TIFF

    Aws::S3::Model::PutObjectRequest object_request;

    for(int i=0; i < 10; i++) {

        object_request.SetBucket(bucketName);

        char name[256];
        snprintf(name, 256, "image%04d.tiff", i);

        object_request.SetKey(name);

        std::cout << "Bucket : " << bucketName << std::endl;
        std::cout << "Object : " << object_request.GetKey() << std::endl;

        std::shared_ptr<Aws::IOStream> aws_stream =
            Aws::MakeShared<Aws::StringStream>("");

        std::ostream *tiff_data = aws_stream.get();

        TIFF *tiff = TIFFStreamOpen("TIFF", tiff_data);
        // TIFF *tiff = TIFFOpen("test.tiff", "w");
        // if (!tiff) {
        //     std::cerr << "Unable to open TIFF file for writing." << std::endl;
        //     return 1;
        // }

        int bitsPerSample = 8, sampleFormat = SAMPLEFORMAT_INT, samplesPerPixel;
        int width = 1024, height = 1024;
        char *image = new char[width * height];

        TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, bitsPerSample);
        TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, sampleFormat);
        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1);
        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, height);
        TIFFSetField(tiff, TIFFTAG_SOFTWARE, "S3 Test");

        unsigned long stripSize;
        stripSize = (unsigned long)TIFFStripSize(tiff);

        int sizeY;
        TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &sizeY);
        TIFFWriteEncodedStrip(tiff, 0, image, stripSize);

        TIFFClose(tiff);

        object_request.SetBody(aws_stream);

        Aws::S3::Model::PutObjectOutcome object_outcome =
            s3_client.PutObject(object_request);

        if (!object_outcome.IsSuccess())
        {
            std::cout << "Error: PutObjectBuffer: " << object_outcome.GetError().GetMessage() << std::endl;
            return false;
        }

        std::cout << "Success: Object written" << std::endl;
    }

    return true;
}

int main()
{
    Aws::Utils::Logging::InitializeAWSLogging(
    Aws::MakeShared<Aws::Utils::Logging::DefaultLogSystem>(
        "RunUnitTests", Aws::Utils::Logging::LogLevel::Trace, "aws_sdk_"));

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::S3::Model::BucketLocationConstraint region =
            Aws::S3::Model::BucketLocationConstraint::us_east_1;

        Aws::String uuid = Aws::Utils::UUID::RandomUUID();
        Aws::String bucket_name = "swilkins/" + 
            Aws::Utils::StringUtils::ToLower(uuid.c_str());

        // Create the bucket.
        CreateTIFF(bucket_name, region);
    }
    ShutdownAPI(options);

    Aws::Utils::Logging::ShutdownAWSLogging();

	return 0;
}