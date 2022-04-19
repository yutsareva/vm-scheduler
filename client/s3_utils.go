package agent_api

import (
	"fmt"
	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/aws/credentials"
	//"github.com/aws/aws-sdk-go/service/s3"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/s3/s3manager"
	"os"
)

type S3Config struct {
	accessKeyId string
	secretAccessKey string
	sessionToken string
	region string
	bucket string
}

func getS3Config() S3Config {
	return S3Config{
		accessKeyId: os.Getenv("VMS_AGENT_AWS_ACCESS_KEY_ID"),
		secretAccessKey: os.Getenv("VMS_AGENT_AWS_SECRET_ACCESS_KEY"),
		region: os.Getenv("VMS_AGENT_AWS_REGION"),
		bucket: os.Getenv("VMS_AGENT_AWS_BUCKET"),
		sessionToken: "",
	}
}

type S3Manager struct {
	config S3Config
}

func (s *S3Manager) session() (*session.Session, error) {
	return session.NewSession(&aws.Config{
		Region: aws.String(s.config.region),
		Credentials: credentials.NewStaticCredentials(
			s.config.accessKeyId, s.config.secretAccessKey, s.config.sessionToken),
	})
}

func (s *S3Manager) uploadFileToS3(fileName *string, key *string) error {
	file, err := os.Open(*fileName)
	if err != nil {
		return err
	}
	defer file.Close()

	session, err := s.session()
	if err != nil {
		return err
	}

	uploader := s3manager.NewUploader(session)

	_, err = uploader.Upload(&s3manager.UploadInput{
		Bucket: aws.String(s.config.bucket),
		Key: aws.String(*key),
		Body: file,
	})
	if err != nil {
		// TODO ("Unable to upload %q to %q, %v", filename, bucket, err)
	}

	fmt.Printf("Successfully uploaded %q to %q\n", key, s.config.bucket)
	return nil
}
