#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <fstream>

int main()
{

  Aws::SDKOptions options;
  Aws::InitAPI(options);
  {
    const Aws::String bucket_name = "sufs-test";
    const Aws::String key_name = "testfile";

    std::cout << "Deleting" << key_name << " from S3 bucket: " <<
      bucket_name << std::endl;

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::Region::US_WEST_2;
    Aws::S3::S3Client s3_client(clientConfig);

    Aws::S3::Model::DeleteObjectRequest object_request;
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto delete_object_outcome = s3_client.DeleteObject(object_request);

    if (delete_object_outcome.IsSuccess())
      {
	std::cout << "Done!" << std::endl;
      }
    else
      {
	std::cout << "DeleteObject error: " <<
	  delete_object_outcome.GetError().GetExceptionName() << " " <<
	  delete_object_outcome.GetError().GetMessage() << std::endl;
      }
  }

  Aws::ShutdownAPI(options);
}
