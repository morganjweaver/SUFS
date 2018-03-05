#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <iostream>
#include <fstream>

/**
 * Put an object from an Amazon S3 bucket.
 * Make sure to use correct KEY and SECRET ACCESS KEY
 */
int main()
{

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    //specify these when loading object onto s3 bucket
    const Aws::String bucket_name = "sufs-test";
    const Aws::String key_name = "testfile";
    const Aws::String file_name = "testfile";

    //print file details
    std::cout << "Filename: " << file_name << std::endl;
    std::cout << "Bucket Name: " << bucket_name << std::endl;
    std::cout << "Key Name: " << key_name << std::endl;

    //configure region
    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::US_WEST_2;
    Aws::S3::S3Client s3_client(clientConfig);

    Aws::S3::Model::PutObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto requestStream  = Aws::MakeShared<Aws::StringStream>("s3");
    //Data should be loaded into this *requestStream
    *requestStream << "Hello World! Please work!";

    object_request.SetBody(requestStream);

    auto put_object_outcome = s3_client.PutObject(object_request);

    if (put_object_outcome.IsSuccess())
      {
	std::cout << "Done!" << std::endl;
      }
    else
      {
	std::cout << "PutObject error: " <<
	  put_object_outcome.GetError().GetExceptionName() << " " <<
	  put_object_outcome.GetError().GetMessage() << std::endl;
      }
  }
  Aws::ShutdownAPI(options);
}
