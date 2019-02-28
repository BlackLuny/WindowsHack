// CryptoApi_Rsa_Test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Windows.h>
#include <WinError.h>


void ShowError(char *pszText)
{
	char szErr[MAX_PATH] = { 0 };
	::wsprintf(szErr, "%s Error[0x%x]\n", pszText, ::GetLastError());
#ifdef _DEBUG
	::MessageBox(NULL, szErr, "ERROR", MB_OK | MB_ICONERROR);
#endif
}


// �������ݵ������ļ�
BOOL SaveDataToFile(char *pszFileName, PVOID pData, DWORD dwDataLength)
{
	BOOL bRet = FALSE;
	HANDLE hFile = NULL;
	DWORD dwFileSize = 0;
	DWORD dwRet = 0;

	do
	{
		hFile = ::CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			break;
		}

		::WriteFile(hFile, pData, dwDataLength, &dwRet, NULL);
		if (dwRet != dwDataLength)
		{
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	if (NULL != hFile)
	{
		::CloseHandle(hFile);
		hFile = NULL;
	}

	return bRet;
}


// ��ȡ�ļ�����
BOOL ReadDataFromFile(char *pszFileName, BYTE **ppData, DWORD *pdwDataLength)
{
	BOOL bRet = FALSE;
	HANDLE hFile = NULL;
	DWORD dwFileSize = 0;
	DWORD dwRet = 0;
	BYTE *pData = NULL;
	DWORD dwDataLength = 0;

	do
	{
		hFile = ::CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			break;
		}

		dwDataLength = ::GetFileSize(hFile, NULL);

		pData = new BYTE[dwDataLength];
		if (NULL == pData)
		{
			break;
		}
		::RtlZeroMemory(pData, dwDataLength);

		::ReadFile(hFile, pData, dwDataLength, &dwRet, NULL);
		if (dwRet != dwDataLength)
		{
			break;
		}

		// ��������
		*ppData = pData;
		*pdwDataLength = dwDataLength;

		bRet = TRUE;

	} while (FALSE);

	if (FALSE == bRet)
	{
		if (NULL != pData)
		{
			delete []pData;
			pData = NULL;
		}
	}

	if (NULL != hFile)
	{
		::CloseHandle(hFile);
		hFile = NULL;
	}

	return bRet;
}



// ���ɹ�Կ��˽Կ
BOOL GenerateKey(BYTE **ppPublicKey, DWORD *pdwPublicKeyLength, BYTE **ppPrivateKey, DWORD *pdwPrivateKeyLength)
{
	BOOL bRet = FALSE;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTKEY hCryptKey = NULL;
	BYTE *pPublicKey = NULL;
	DWORD dwPublicKeyLength = 0;
	BYTE *pPrivateKey = NULL;
	DWORD dwPrivateKeyLength = 0;

	do
	{
		// ��ȡCSP���
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0);
		if (NTE_BAD_KEYSET == ::GetLastError())
		{
			bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET);
			if (FALSE == bRet)
			{
				ShowError("CryptAcquireContext");
				break;
			}
		}

		// ���ɹ�˽��Կ��
		bRet = ::CryptGenKey(hCryptProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &hCryptKey);
		if (FALSE == bRet)
		{
			ShowError("CryptGenKey");
			break;
		}

		// ��ȡ��Կ��Կ�ĳ��Ⱥ�����
		bRet = ::CryptExportKey(hCryptKey, NULL, PUBLICKEYBLOB, 0, NULL, &dwPublicKeyLength);
		if (FALSE == bRet)
		{
			ShowError("CryptExportKey");
			break;
		}
		pPublicKey = new BYTE[dwPublicKeyLength];
		::RtlZeroMemory(pPublicKey, dwPublicKeyLength);
		bRet = ::CryptExportKey(hCryptKey, NULL, PUBLICKEYBLOB, 0, pPublicKey, &dwPublicKeyLength);
		if (FALSE == bRet)
		{
			ShowError("CryptExportKey");
			break;
		}

		// ��ȡ˽Կ��Կ�ĳ��Ⱥ�����
		bRet = ::CryptExportKey(hCryptKey, NULL, PRIVATEKEYBLOB, 0, NULL, &dwPrivateKeyLength);
		if (FALSE == bRet)
		{
			ShowError("CryptExportKey");
			break;
		}
		pPrivateKey = new BYTE[dwPrivateKeyLength];
		::RtlZeroMemory(pPrivateKey, dwPrivateKeyLength);
		bRet = ::CryptExportKey(hCryptKey, NULL, PRIVATEKEYBLOB, 0, pPrivateKey, &dwPrivateKeyLength);
		if (FALSE == bRet)
		{
			ShowError("CryptExportKey");
			break;
		}

		// ��������
		*ppPublicKey = pPublicKey;
		*pdwPublicKeyLength = dwPublicKeyLength;
		*ppPrivateKey = pPrivateKey;
		*pdwPrivateKeyLength = dwPrivateKeyLength;

	} while (FALSE);

	// �ͷŹر�
	if (hCryptKey)
	{
		::CryptDestroyKey(hCryptKey);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}


// ��Կ��������
BOOL RsaEncrypt(BYTE *pPublicKey, DWORD dwPublicKeyLength, BYTE *pData, DWORD &dwDataLength, DWORD dwBufferLength)
{
	BOOL bRet = FALSE;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTKEY hCryptKey = NULL;

	do
	{
		// ��ȡCSP���
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0);
		if (NTE_BAD_KEYSET == ::GetLastError())
		{
			bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET);
			if (FALSE == bRet)
			{
				ShowError("CryptAcquireContext");
				break;
			}
		}

		// ���빫Կ
		bRet = ::CryptImportKey(hCryptProv, pPublicKey, dwPublicKeyLength, NULL, 0, &hCryptKey);
		if (FALSE == bRet)
		{
			ShowError("CryptImportKey");
			break;
		}

		// ��������
		bRet = ::CryptEncrypt(hCryptKey, NULL, TRUE, 0, pData, &dwDataLength, dwBufferLength);
		if (FALSE == bRet)
		{
			ShowError("CryptImportKey");
			break;
		}

	} while (FALSE);

	// �ͷŲ��ر�
	if (hCryptKey)
	{
		::CryptDestroyKey(hCryptKey);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}


// ˽Կ��������
BOOL RsaDecrypt(BYTE *pPrivateKey, DWORD dwProvateKeyLength, BYTE *pData, DWORD &dwDataLength)
{
	BOOL bRet = FALSE;
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTKEY hCryptKey = NULL;

	do
	{
		// ��ȡCSP���
		bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0);
		if (NTE_BAD_KEYSET == ::GetLastError())
		{
			bRet = ::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET);
			if (FALSE == bRet)
			{
				ShowError("CryptAcquireContext");
				break;
			}
		}

		// ����˽Կ
		bRet = ::CryptImportKey(hCryptProv, pPrivateKey, dwProvateKeyLength, NULL, 0, &hCryptKey);
		if (FALSE == bRet)
		{
			ShowError("CryptImportKey");
			break;
		}

		// ��������
		bRet = ::CryptDecrypt(hCryptKey, NULL, TRUE, 0, pData, &dwDataLength);
		if (FALSE == bRet)
		{
			ShowError("CryptDecrypt");
			break;
		}

	} while (FALSE);

	// �ͷŲ��ر�
	if (hCryptKey)
	{
		::CryptDestroyKey(hCryptKey);
	}
	if (hCryptProv)
	{
		::CryptReleaseContext(hCryptProv, 0);
	}

	return bRet;
}



int _tmain(int argc, _TCHAR* argv[])
{
	int n = 0;
	BOOL bRet = FALSE;
	BYTE *pPublicKey = NULL;
	DWORD dwPublicKeyLength = 0;
	BYTE *pPrivateKey = NULL;
	DWORD dwPrivateKeyLength = 0;
	BYTE *pData = NULL;
	DWORD dwTextLength = 0;
	DWORD dwDataLength = 4096;
	DWORD dwBufferLength = 4096;
	DWORD i = 0;

	char szFileName[MAX_PATH] = { 0 };
	pData = new BYTE[dwDataLength];

	::RtlZeroMemory(pData, dwDataLength);
	::RtlZeroMemory(szFileName, MAX_PATH);


	do
	{
		printf("----------------- Win32 Crypto Api Test -----------------\n");
		printf("1. ���ɹ�˽��Կ��, �����浽�����ļ�\n");
		printf("2. ��Կ��������\n");
		printf("3. ˽Կ��������\n");
		printf("---------------------------------------------------------\n");

		scanf("%d", &n);
		getchar();

		switch (n)
		{
		case 1:
		{
			// ���ɹ�Կ��˽Կ
			bRet = GenerateKey(&pPublicKey, &dwPublicKeyLength, &pPrivateKey, &dwPrivateKeyLength);
			if (FALSE == bRet)
			{
				exit(0);
				break;
			}
			SaveDataToFile("public.enc", pPublicKey, dwPublicKeyLength);
			SaveDataToFile("private.enc", pPrivateKey, dwPrivateKeyLength);

			break;
		}
		case 2:
		{
			::RtlZeroMemory(pData, dwDataLength);
			::RtlZeroMemory(szFileName, MAX_PATH);

			printf("����Ҫ���ܵ��ַ���:\n");
			gets((char *)pData);

			printf("���빫Կ�ļ�·��:\n");
			gets(szFileName);
			bRet = ReadDataFromFile(szFileName, &pPublicKey, &dwPublicKeyLength);
			if (FALSE == bRet)
			{
				exit(0);
				break;
			}

			// ����
			dwTextLength = 1 + ::lstrlen((char *)pData);
			bRet = RsaEncrypt(pPublicKey, dwPublicKeyLength, pData, dwTextLength, dwBufferLength);
			if (FALSE == bRet)
			{
				exit(0);
				break;
			}

			// �������ĵ��ļ�
			SaveDataToFile("encdata.enc", pData, dwTextLength);

			break;
		}
		case 3:
		{
			::RtlZeroMemory(pData, dwDataLength);
			::RtlZeroMemory(szFileName, MAX_PATH);

			printf("��������·��:\n");
			gets(szFileName);
			bRet = ReadDataFromFile(szFileName, &pData, &dwTextLength);
			if (FALSE == bRet)
			{
				exit(0);
				break;
			}

			printf("����˽Կ�ļ�·��:\n");
			gets(szFileName);
			bRet = ReadDataFromFile(szFileName, &pPrivateKey, &dwPrivateKeyLength);
			if (FALSE == bRet)
			{
				exit(0);
				break;
			}

			bRet = RsaDecrypt(pPrivateKey, dwPrivateKeyLength, pData, dwTextLength);
			if (FALSE == bRet)
			{
				exit(0);
				break;
			}

			// ��ʾ
			printf("���ܽ��Ϊ:\n");
			printf("%s", (char *)pData);
			printf("\n\n\n\n");

			break;
		}
		default:
		{
			printf("input error!\n");
			break;
		}
		}
		
	} while (0 != n);

	// �ͷ�
	if (pData)
	{
		delete[]pData;
		pData = NULL;
	}
	if (pPrivateKey)
	{
		delete[]pPrivateKey;
		pPrivateKey = NULL;
	}
	if (pPublicKey)
	{
		delete[]pPublicKey;
		pPublicKey = NULL;
	}

	system("pause");
	return 0;
}

