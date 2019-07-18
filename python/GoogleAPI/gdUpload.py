from __future__ import print_function
import pickle
import argparse
import os.path
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request
import googleapiclient.http

# If modifying these scopes, delete the file token.pickle.
SCOPES = ['https://www.googleapis.com/auth/drive.file']

def main(filePath, fileName, mimeType, folderID, fileCredential):
    """Shows basic usage of the Drive v3 API.
    Prints the names and ids of the first 10 files the user has access to.
    """
    creds = None
    # The file token.pickle stores the user's access and refresh tokens, and is
    # created automatically when the authorization flow completes for the first
    # time.
    if os.path.exists('token.pickle'):
        with open('token.pickle', 'rb') as token:
            creds = pickle.load(token)
    # If there are no (valid) credentials available, let the user log in.
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(fileCredential, SCOPES)
            creds = flow.run_local_server(port=0)
        # Save the credentials for the next run
        with open('token.pickle', 'wb') as token:
            pickle.dump(creds, token)

    service = build('drive', 'v3', credentials=creds)

    file_metadata = {'name': fileName, 'parents': [folderID]}
    media = googleapiclient.http.MediaFileUpload(filePath, mimetype=mimeType)
    file = service.files().create(body=file_metadata,
                                        media_body=media,
                                        fields='id').execute()
    print ('File ID: %s' % file.get('id'))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Google Drive uploader')
    parser.add_argument('filePath',   help='path to a file')
    parser.add_argument('fileName',   help='file name')
    parser.add_argument('mimeType',   help='mimie type')
    parser.add_argument('folderID',   help='folder ID in Google Drive')
    parser.add_argument('credentials',   help='credentials.json')

    args = vars(parser.parse_args())

    main(args['filePath'], args['fileName'], args['mimeType'], args['folderID'], args['credentials'])
