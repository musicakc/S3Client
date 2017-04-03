/*
 * for creds set/export AWS_ACCESS_KEY_ID AWS_SECRET_ACCESS_KEY
 * and pass -e = localhost:8000 if using vstart.sh 
 */

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/Bucket.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <network/uri.hpp>

const char *my_endpoint;
const char *my_region;

int exitcode;

int process()
{
	Aws::Client::ClientConfiguration clientconfig;
	if (my_region)
		clientconfig.region = my_region;
	if (my_endpoint) {
		Aws::String aws_my_endpoint(my_endpoint);
		network::uri u(my_endpoint);
		auto us = u.scheme().to_string();
		if (!us.compare("http")) {
			clientconfig.scheme = Aws::Http::Scheme::HTTP;
		}
		else if (!us.compare("https")) {
			clientconfig.scheme = Aws::Http::Scheme::HTTPS;
		} else {
			throw std::out_of_range("invalid scheme " + us);
		}
		std::string ur;
		ur = u.host().to_string();
		if (u.has_port()) {
			ur += ":" + u.port().to_string();
		}
		clientconfig.endpointOverride = ur.c_str();
	}

	Aws::S3::S3Client s3_client(clientconfig);
	
	//Create Bucket
	Aws::S3::Model::CreateBucketRequest create_bucket;
        create_bucket.WithBucket("new_bucket");

        auto create_bucket_outcome = s3_client.CreateBucket(create_bucket);

        if (create_bucket_outcome.IsSuccess()) {
            std::cout << "Done!" << std::endl;
        } else {
            std::cout << "CreateBucket error: " <<
                create_bucket_outcome.GetError().GetExceptionName() << std::endl
                << create_bucket_outcome.GetError().GetMessage() << std::endl;
        }

	//Delete Bucket
	Aws::S3::Model::DeleteBucketRequest del_bucket;
        del_bucket.WithBucket("new bucket");

        auto delete_bucket_outcome = s3_client.DeleteBucket(del_bucket);

        if (delete_bucket_outcome.IsSuccess()) {
            std::cout << "Done!" << std::endl;
        } else {
            std::cout << "DeleteBucket error: " <<
                delete_bucket_outcome.GetError().GetExceptionName() << std::endl
                << delete_bucket_outcome.GetError().GetMessage() << std::endl;
        }

	
	//Copy Object
	Aws::S3::Model::CopyObjectRequest object_request;
        object_request.WithBucket("bucket_1")
                      .WithKey("random key")
                      .WithCopySource(std::string("bucket_2") + std::string("random_key"));

        auto copy_object_outcome = s3_client.CopyObject(object_request);

        if (copy_object_outcome.IsSuccess()) {
            std::cout << "Done!" << std::endl;
        } else {
            std::cout << "CopyObject error: " <<
                copy_object_outcome.GetError().GetExceptionName() << " " <<
                copy_object_outcome.GetError().GetMessage() << std::endl;
        }
}

int main(int ac, char **av)
{
	char *ap;
	while (--ac > 0) if (*(ap = *++av) == '-') while (*++ap) switch(*ap) {
	case 'e':
		if (--ac <= 0) {
			std::cerr << "-e: missing endpoint " << std::endl;
			goto Usage;
		}
		my_endpoint = *++av;
		my_region = "mexico";
		break;
	case '-':
		break;
	default:
		std::cerr << "bad flag " << *ap << std::endl;
	Usage:
		std::cerr << "Usage: lb2 [-e endpoint]" << std::endl;
		exit(1);
	}
	{
		Aws::SDKOptions options;
		Aws::InitAPI(options);
		process();
		Aws::ShutdownAPI(options);
	}
	exit(exitcode);
}
