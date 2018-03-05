#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CopyObjectRequest.h>
#include <fstream>

/**
 * List objects (keys) within an Amazon S3 bucket.
 */
int main()
{
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    //Set these variables before copying
    const Aws::String key_name = "testfile";
    const Aws::String from_bucket = "sufs-test";
    const Aws::String to_bucket = "sufs-test2";

    std::cout << "Copying" << key_name << " from bucket " << from_bucket <<
      " to " << to_bucket << std::endl;

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::US_WEST_2;
    Aws::S3::S3Client s3_client(clientConfig);

    Aws::S3::Model::CopyObjectRequest object_request;
    object_request.WithBucket(to_bucket)
      .WithKey(key_name)
      .WithCopySource(from_bucket + "/" + key_name);

    auto copy_object_outcome = s3_client.CopyObject(object_request);

    if (copy_object_outcome.IsSuccess())
      {
	std::cout << "Done!" << std::endl;
      }
    else
      {
	std::cout << "CopyObject error: " <<
	  copy_object_outcome.GetError().GetExceptionName() << " " <<
	  copy_object_outcome.GetError().GetMessage() << std::endl;
      }
  }

  Aws::ShutdownAPI(options);
}
