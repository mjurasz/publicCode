#include "stdafx.h"
#include "tec2fileoperator.h"
#include "TEC2_INSPEKCJAREC.h"
#include "GeoTec.h"
#include "GeoTecDoc.h"

CTEC2FileOperator :: CTEC2FileOperator(void)
{
	m_POINT_EVENTS_MASK_NEG		= 0x0;
	m_nCurrModalEventsMask		= 0x0;
	m_nNbOfStoredInspectionRecords = 0;
	m_nNbOfStoredMeasRecords	= 0;
}

CTEC2FileOperator :: ~CTEC2FileOperator(void)
{
	m_nNbOfStoredMeasRecords = 0;
	m_nNbOfStoredInspectionRecords = 0;
	m_oaFileRecs.RemoveAll();
	m_oaInspekcjaFileRecs.RemoveAll();
	m_nSectionID = 0;
	m_nCurrModalEventsMask = 0x0;
}

//////////////////////////////////////////////////////////////////////////////////
// Funkcja otwiera plik binarny z TEC2, odczytuje naglowek i informacje o pomiarze
// - tylko podstawowe informacje - bez danych
BOOL CTEC2FileOperator :: TEC2_ReadMeasurementFileHeader(int &nVersion, CString strFileName, _MEASFILEINFO &strctMeasInfo)
{
	CFileException oException;
CFile	oBinFile;
	int		nPacketCode = 0, nPacketSize = 0;

	try
	{
		/* otwieramy plik zadany parametrem strFileName */
		if (!oBinFile.Open(strFileName, CFile :: modeRead | CFile :: shareExclusive, &oException))
		{
			#ifdef _DEBUG
				oException.ReportError();
			#endif
			return FALSE;
		}

		/* przesuwamy sie do naglowka pliku pomiarowego */
		while (TRUE)
		{
			/* pobieramy z zadanego pliku wielkosc pakietu i kod */
			if (!GetFilePacketCodeAndSize(oBinFile, nPacketSize, nPacketCode))
			{
				oBinFile.Close();
				return FALSE;
			}

			/* sprawdzamy czy pobrany kod jest jednym z rozpoznawanych */
			if (!ValidateFCodeFromFile(nPacketCode))
			{
				oBinFile.Close();
				return FALSE;
			}

			/* tutaj chcemy sie dowiedziec o wersje pliku - czy jest z nowego TEC'a - jesli nie, to pomijamy plik */
			if (nPacketCode == FCODE_SYGNATURA)
			{
				/* odczytujemy pakiet */
				if (!ReadFilePacket(oBinFile, m_chFilePacketBuffor, nPacketSize))
				{
					oBinFile.Close();
					return FALSE;
				}

				CString strTRT;
				strTRT.Format("%s", m_chFilePacketBuffor);

				if (strTRT.Compare("TRT002"))
					return FALSE;
				strctMeasInfo.nMeasDev = 0;

				continue;
			}

			/* jesli to nie jest pakiet oznaczajacy naglowek pliku torowego */
			if (nPacketCode != FCODE_NAGL_TOR)
			{
				/* przesuwamy "kursor" odczytu pliku odebranego z TEC2 o zadana liczne bajtow */
				if (!SkipPartOfFile(oBinFile, nPacketSize))
				{
					oBinFile.Close();
					return FALSE;
				}
			}
			else break;
        }

		/* odczytujemy pakiet */
		if (!ReadFilePacket(oBinFile, m_chFilePacketBuffor, nPacketSize))
		{
			oBinFile.Close();
			return FALSE;
		}

		ASSERT(nPacketSize >= 0 && nPacketSize < FILE_PACKET_BUFFOR_SIZE);

		/* pobieramy naglowek na podstawie pakietu z sygnatura zadanego parametrem */
		if (!GetMeasurementHeaderInformations(m_chFilePacketBuffor, nPacketSize, nVersion))
		{
			oBinFile.Close();
			return FALSE;
		}

		oBinFile.Close();
	}
	catch(CException* pErr)
	{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif

		if (oBinFile.m_hFile != NULL && oBinFile != CFile :: hFileNull)
			oBinFile.Close();

		pErr->Delete();
		return FALSE;
	}

	m_strctTEC2MeasHeader.strFileName = strFileName;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////
// Funkcja czyta plik pomiarowy - przesuwa sie do danych pomiarowych i w przypadku
// odczytania odpowiedniego kodu (obslugiwanego) wywoluje funkcje ktora wykonuje z danymi
// stosowne operacje
BOOL CTEC2FileOperator :: TEC2_ReadMeasurementData(int &nVersion, CString strFileName, _MEASFILEINFO &strctMeasInfo) 
{
	CFileException oException;
	BOOL	rv;
	CFile	oBinFile;
	int		nPacketCode = 0, nPacketSize = 0;
	double	nfCurrKm = 0.0, nfCurrStep = 0.0;
	BOOL	bCurrDirectionIncreasing = FALSE;
	int		nNbOfValidMeasRecordsInBuffer = 0;
	BOOL	bCounterDisabled = FALSE;

	if (!bCurrDirectionIncreasing)
		nfCurrStep *= (-1);

	bCounterDisabled = (m_strctTEC2MeasHeader.nCountingType == ROAD_COUNTER_DISABLED);

	m_nNbOfStoredMeasRecords	= 0;
	m_nNbOfStoredInspectionRecords = 0;

	for (int i = 0; i < m_nLiczbaParametrow; i++)
		m_oaDataRecord[i] = NODRAWFLAG;

	m_strctFileRecs[0]			= TEC2_GetDefaultDocFileMeasRec();
	nfCurrKm					= m_strctTEC2MeasHeader.nfBegKm;
	nfCurrStep					= m_strctTEC2MeasHeader.nfMeasStep;
	bCurrDirectionIncreasing	= m_strctTEC2MeasHeader.bDirectionInc;
	m_strctFileRecs[0].m_nfKm		= nfCurrKm;
	m_nLastFCode				= 0xFFFFFFFF;
	m_nSectionID				= 1;

	m_POINT_EVENTS_MASK_NEG = GetMaskOfPoinsEvents();
	m_POINT_EVENTS_MASK_NEG = (0xFFFFFFFF ^ m_POINT_EVENTS_MASK_NEG);

	if (bCounterDisabled == TRUE)
	{
		m_strctFileRecs[0].m_nSectionID = m_nSectionID;
		m_strctFileRecs[0].m_nfKm = 0.0;
	}
	else
	{
		m_strctFileRecs[0].m_nSectionID = m_nSectionID;
		m_strctFileRecs[0].m_nfKm = nfCurrKm;
	}

	try
	{
		/* otwieramy plik zadany parametrem strFileName */
		if (!oBinFile.Open(strFileName, CFile :: modeRead | CFile :: shareExclusive, &oException))
		{
			#ifdef _DEBUG
				oException.ReportError();
			#endif
			return FALSE;
		}

		/* przesuwamy sie do naglowka pliku pomiarowego */
		while (TRUE)
		{
			/* pobieramy z zadanego pliku wielkosc pakietu i kod */
			if (!GetFilePacketCodeAndSize(oBinFile, nPacketSize, nPacketCode))
			{
				oBinFile.Close();
				return FALSE;
			}

			/* sprawdzamy czy pobrany kod jest jednym z rozpoznawanych */
			if (!ValidateFCodeFromFile(nPacketCode))
			{
				oBinFile.Close();
				return FALSE;
			}

			if (nPacketCode != FCODE_NAGL_TOR)
			{
				/* przesuwamy "kursor" odczytu pliku odebranego z TEC2 o zadana liczne bajtow */
				if (!SkipPartOfFile(oBinFile, nPacketSize))
				{
					oBinFile.Close();
					return FALSE;
				}
			}
			else break;
        }

		ASSERT(nPacketSize >= 0 && nPacketSize < FILE_PACKET_BUFFOR_SIZE);

		/* pobieramy naglowek na podstawie pakietu z sygnatura zadanego parametrem */
		if (!GetMeasurementHeaderInformations(m_chFilePacketBuffor, nPacketSize, nVersion))
		{
			oBinFile.Close();
			return FALSE;
		}

		/* pomijamy naglowek */
		if (!SkipPartOfFile(oBinFile, nPacketSize))
		{
			oBinFile.Close();
			return FALSE;
		}

		/* doszlismy do danych */
		TRACE("Czytamy paczki danych:\n");
		/* do weryfikacji */
		do
		{
			/* tak na wszelki wypadek zerujemy - TEST MJ - powinno byc odblokowane */
			nPacketSize = 0;
			/* pobieramy z zadanego pliku wielkosc pakietu i kod */
			if (!GetFilePacketCodeAndSize(oBinFile, nPacketSize, nPacketCode))
			{
				TRACE("Blad!! Kod: %x, Rozmiar: %d\n", nPacketCode, nPacketSize);
				/* jesli rozmiar pakietu wiekszy od zera - cos jest nie tak */
				if (nPacketSize > 0)
					return FALSE;

				/* cos bylo nie tak, ale mamy w buforze jakies dane, wiec zapisujemy do pliku */
				if (nNbOfValidMeasRecordsInBuffer > 0)
					if (TEC2_WriteToBuffer(nNbOfValidMeasRecordsInBuffer) == FALSE)
						return FALSE;

				nNbOfValidMeasRecordsInBuffer = 0;

				VERIFY(m_nNbOfStoredMeasRecords > 0);

				/* czy odczytano cokolwiek */
				if (m_nNbOfStoredMeasRecords == 0)
					return FALSE;

				return TRUE;
			}

			/* sprawdzamy czy pobrany kod jest jednym z rozpoznawanych */
			if (!ValidateFCodeFromFile(nPacketCode))
			{
				/* przesuwamy kursor odczytu pliku z DTG o zadana liczne bajtow */
				if (!SkipPartOfFile(oBinFile, nPacketSize))
					return FALSE;

				rv = TRUE;

				continue;
			}

			/* pobiera z zadanego pliku wielkosc pakietu i kod - czytamy dane */
			if (!ReadFilePacket(oBinFile, m_chFilePacketBuffor, nPacketSize))
				return FALSE;

			/* konwertujemy odczytany pakiet do postaci wlasciwej danej w rekordzie pomiarowym (³¹cznie z inspekcj¹) */
			if (!TEC2_ConvertReceivedData(m_pDoc, m_chFilePacketBuffor, nPacketCode, nPacketSize, bCounterDisabled,
									     nfCurrKm, nfCurrStep, bCurrDirectionIncreasing, nNbOfValidMeasRecordsInBuffer))
				return FALSE;

			TRACE("Km: %.4f, k: %.4f, naras. %d, events %x, ilosc: %d\n", nfCurrKm, nfCurrStep, bCurrDirectionIncreasing, 
				m_strctFileRecs[nNbOfValidMeasRecordsInBuffer - 1].m_events, nNbOfValidMeasRecordsInBuffer);		

			/* jesli odczytalismy maksymalna dozwolona liczbe rekordow pomiarowych */
			if (nNbOfValidMeasRecordsInBuffer == CONVERT_TEC2RECORDS_BUFFER_SIZE - 1)
			{
				/* zapisujemy wszystkie dotychczas odczytane */
				if (!TEC2_WriteToBuffer(nNbOfValidMeasRecordsInBuffer))
					return FALSE;

				// po co?
				m_strctFileRecs[0] = m_strctFileRecs[nNbOfValidMeasRecordsInBuffer];

				nNbOfValidMeasRecordsInBuffer = 0;
			}
		} while (rv == TRUE);
		/* do weryfikacji */

		oBinFile.Close();
	}
	catch(CException* pErr)
	{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif

		if (oBinFile.m_hFile != NULL && oBinFile != CFile :: hFileNull)
			oBinFile.Close();

		pErr->Delete();
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja zapisuje dane pomiarowe z pliku odebranego z DTG do pliku w formacie Visaul-a
// Parametry:
// strDocFileName - nazwa pliku w formacie Visual-a
BOOL CTEC2FileOperator :: StoreMeasDataInDoc(CGeoTecDoc* pDoc, CString strDocFileName)
{
	CFileException oException;
	BOOL	rv;
	CFile	oBinFile;
	int		nPacketCode = 0, nPacketSize = 0;
	double	nfCurrKm = 0.0, nfCurrStep = 0.0;
	BOOL	bCurrDirectionIncreasing = FALSE;
	int		nNbOfValidMeasRecordsInBuffer = 0;
	BOOL	bCounterDisabled = FALSE;

	int nVersion = -1;

	if (!bCurrDirectionIncreasing)
		nfCurrStep *= (-1);

	bCounterDisabled = (m_strctTEC2MeasHeader.nCountingType == ROAD_COUNTER_DISABLED);

	m_nNbOfStoredMeasRecords	= 0;
	m_nNbOfStoredInspectionRecords = 0;

	for (int i = 0; i < m_nLiczbaParametrow; i++)
		m_oaDataRecord[i] = NODRAWFLAG;

	m_strctFileRecs[0]			= TEC2_GetDefaultDocFileMeasRec();
	nfCurrKm					= m_strctTEC2MeasHeader.nfBegKm;
	nfCurrStep					= m_strctTEC2MeasHeader.nfMeasStep;
	bCurrDirectionIncreasing	= m_strctTEC2MeasHeader.bDirectionInc;
	m_strctFileRecs[0].m_nfKm		= nfCurrKm;
	m_nLastFCode				= 0xFFFFFFFF;
	m_nSectionID				= 1;

	m_POINT_EVENTS_MASK_NEG = GetMaskOfPoinsEvents();
	m_POINT_EVENTS_MASK_NEG = (0xFFFFFFFF ^ m_POINT_EVENTS_MASK_NEG);

	if (bCounterDisabled == TRUE)
	{
		m_strctFileRecs[0].m_nSectionID = m_nSectionID;
		m_strctFileRecs[0].m_nfKm = 0.0;
	}
	else
	{
		m_strctFileRecs[0].m_nSectionID = m_nSectionID;
		m_strctFileRecs[0].m_nfKm = nfCurrKm;
	}

	try
	{
		/* otwieramy plik zadany parametrem strFileName */
		if (!oBinFile.Open(pDoc->m_oTEC2FileOperator.m_strctTEC2MeasHeader.strFileName, CFile :: modeRead | CFile :: shareExclusive, &oException))
		{
			#ifdef _DEBUG
				oException.ReportError();
			#endif
			return FALSE;
		}

		if (!m_oOutputFile.CreateStream(pDoc->GetRootStream(), strDocFileName, CFile :: modeReadWrite | CFile :: shareExclusive | CFile :: modeCreate, &oException))
		{
			if (oBinFile.m_hFile != CFile :: hFileNull && oBinFile.m_hFile != NULL)
				oBinFile.Close();

			return FALSE;
		}

		/* to powoduje bledy */
		/* CString strFileName;
		for (int i = 0; i < NB_OF_EVENTS; i++)
		{
			strFileName.Format("%s%d", MEASEVENTFILE, i);
			rv = m_oaEventWriter[i].Initialize(pDoc->GetRootStream(), strFileName, i, TRUE, g_nMileageStoringScale);
			if (rv != RGOK)
				return FALSE;
		} */

		/* przesuwamy sie do naglowka pliku pomiarowego */
		while (TRUE)
		{
			/* pobieramy z zadanego pliku wielkosc pakietu i kod */
			if (!GetFilePacketCodeAndSize(oBinFile, nPacketSize, nPacketCode))
			{
				oBinFile.Close();
				return FALSE;
			}

			/* sprawdzamy czy pobrany kod jest jednym z rozpoznawanych */
			if (!ValidateFCodeFromFile(nPacketCode))
			{
				oBinFile.Close();
				return FALSE;
			}

			if (nPacketCode != FCODE_NAGL_TOR)
			{
				/* przesuwamy "kursor" odczytu pliku odebranego z TEC2 o zadana liczne bajtow */
				if (!SkipPartOfFile(oBinFile, nPacketSize))
				{
					oBinFile.Close();
					return FALSE;
				}
			}
			else break;
        }

		ASSERT(nPacketSize >= 0 && nPacketSize < FILE_PACKET_BUFFOR_SIZE);

		/* pobieramy naglowek na podstawie pakietu z sygnatura zadanego parametrem */
		if (!GetMeasurementHeaderInformations(m_chFilePacketBuffor, nPacketSize, nVersion))
		{
			oBinFile.Close();
			return FALSE;
		}

		/* pomijamy naglowek */
		if (!SkipPartOfFile(oBinFile, nPacketSize))
		{
			oBinFile.Close();
			return FALSE;
		}

		/* doszlismy do danych */
		TRACE("Czytamy paczki danych:\n");
		/* do weryfikacji */
		do
		{
			/* tak na wszelki wypadek zerujemy - TEST MJ - powinno byc odblokowane */
			nPacketSize = 0;
			/* pobieramy z zadanego pliku wielkosc pakietu i kod */
			if (!GetFilePacketCodeAndSize(oBinFile, nPacketSize, nPacketCode))
			{
				TRACE("Blad!! Kod: %x, Rozmiar: %d\n", nPacketCode, nPacketSize);
				/* jesli rozmiar pakietu wiekszy od zera - cos jest nie tak */
				// if (nPacketSize > 0)
				//	return FALSE;

				/* cos bylo nie tak, ale mamy w buforze jakies dane, wiec zapisujemy do pliku */
				if (nNbOfValidMeasRecordsInBuffer > 0)
					if (WriteBufferToFile(nNbOfValidMeasRecordsInBuffer) == FALSE)
						return FALSE;

				nNbOfValidMeasRecordsInBuffer = 0;

				VERIFY(m_nNbOfStoredMeasRecords > 0);

				/* czy odczytano cokolwiek */
				if (m_nNbOfStoredMeasRecords == 0)
					return FALSE;

				pDoc->m_oRouteInfo.UpdateLastSector(nfCurrKm, m_nNbOfStoredMeasRecords);

				return TRUE;
			}

			/* sprawdzamy czy pobrany kod jest jednym z rozpoznawanych */
			if (!ValidateFCodeFromFile(nPacketCode))
			{
				/* przesuwamy kursor odczytu pliku z DTG o zadana liczne bajtow */
				if (!SkipPartOfFile(oBinFile, nPacketSize))
					return FALSE;

				rv = TRUE;

				continue;
			}

			/* pobiera z zadanego pliku wielkosc pakietu i kod - czytamy dane */
			if (!ReadFilePacket(oBinFile, m_chFilePacketBuffor, nPacketSize))
				return FALSE;

			/* konwertujemy odczytany pakiet do postaci wlasciwej danej w rekordzie pomiarowym (³¹cznie z inspekcj¹) */
			if (!TEC2_ConvertReceivedData(pDoc, m_chFilePacketBuffor, nPacketCode, nPacketSize, bCounterDisabled,
									     nfCurrKm, nfCurrStep, bCurrDirectionIncreasing, nNbOfValidMeasRecordsInBuffer))
				return FALSE;

			TRACE("Km: %.4f, k: %.4f, naras. %d, events %x, ilosc: %d\n", nfCurrKm, nfCurrStep, bCurrDirectionIncreasing, 
				m_strctFileRecs[nNbOfValidMeasRecordsInBuffer - 1].m_events, nNbOfValidMeasRecordsInBuffer);		

			/* jesli odczytalismy maksymalna dozwolona liczbe rekordow pomiarowych */
			if (nNbOfValidMeasRecordsInBuffer == CONVERT_TEC2RECORDS_BUFFER_SIZE - 1)
			{
				/* zapisujemy wszystkie dotychczas odczytane */
				if (!WriteBufferToFile(nNbOfValidMeasRecordsInBuffer))
				{
					if (m_oOutputFile.GetStream() != NULL)
						m_oOutputFile.Close();
					if (oBinFile.m_hFile != CFile :: hFileNull && oBinFile.m_hFile != NULL)
						oBinFile.Close();
					return FALSE;
				}

				// po co?
				m_strctFileRecs[0] = m_strctFileRecs[nNbOfValidMeasRecordsInBuffer];

				nNbOfValidMeasRecordsInBuffer = 0;
			}
		} while (rv == TRUE);
		/* do weryfikacji */

		oBinFile.Close();
	}
	catch(CException* pErr)
	{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif

		if (m_oOutputFile.GetStream() != NULL)
			m_oOutputFile.Close();
		if (oBinFile.m_hFile != NULL && oBinFile != CFile :: hFileNull)
			oBinFile.Close();

		pErr->Delete();
		return FALSE;
	}

	if (nNbOfValidMeasRecordsInBuffer > 0)
		if (WriteBufferToFile(nNbOfValidMeasRecordsInBuffer) == FALSE)
		{
			if (m_oOutputFile.GetStream() != NULL)
				m_oOutputFile.Close();
			if (oBinFile.m_hFile != CFile :: hFileNull && oBinFile.m_hFile != NULL)
				oBinFile.Close();
			return FALSE;
		}
	/* to powoduje bledy */
	//for (int i = 0; i < NB_OF_EVENTS; i++)
	//	m_oaEventWriter[i].Terminate(); 

	if (m_oOutputFile.GetStream() != NULL)
		m_oOutputFile.Close();

	if (oBinFile.m_hFile != CFile :: hFileNull && oBinFile.m_hFile != NULL)
		oBinFile.Close();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja zapisuje bufor do pliku visual-a
// Parametry:
// nNbOfValidMeasRecordsInBuffer - liczba poprawnych punktow pomiarowych w buforze
BOOL CTEC2FileOperator :: WriteBufferToFile(int nNbOfValidMeasRecordsInBuffer)
{
	UINT nBytesToWrite;
	ASSERT(nNbOfValidMeasRecordsInBuffer > 0 && nNbOfValidMeasRecordsInBuffer < CONVERT_TEC2RECORDS_BUFFER_SIZE);

	for (int i = 0; i < nNbOfValidMeasRecordsInBuffer; i++)
	{
		m_strctFileRecsBuffer[i].width = m_strctFileRecs[i].m_width;
		m_strctFileRecsBuffer[i].cant = m_strctFileRecs[i].m_cant;
		m_strctFileRecsBuffer[i].horizIrrL = m_strctFileRecs[i].m_horizIrrL;
		m_strctFileRecsBuffer[i].vertIrrL = m_strctFileRecs[i].m_vertIrrL;
		m_strctFileRecsBuffer[i].events = m_strctFileRecs[i].m_events;
		m_strctFileRecsBuffer[i].nfKm = m_strctFileRecs[i].m_nfKm;
		m_strctFileRecsBuffer[i].dbevents = m_strctFileRecs[i].m_dbevents;
		m_strctFileRecsBuffer[i].nFlags = m_strctFileRecs[i].m_nFlags;
		m_strctFileRecsBuffer[i].nSectionID = m_strctFileRecs[i].m_nSectionID;

#ifdef COMPILE_MC
		m_strctFileRecsBuffer[i].horizIrrR = 0;
		m_strctFileRecsBuffer[i].vertIrrR = 0;
#endif

#ifdef VERSION_SUBWAY_PL
		m_strctFileRecsBuffer[i].horizSupRail = 0;
		m_strctFileRecsBuffer[i].vertSupRail = 0;
#endif

#ifdef TEC2WITHGROOVES
		m_strctFileRecsBuffer[i].nGrooveLeft = m_strctFileRecs[i].m_nGrooveLeft;
		m_strctFileRecsBuffer[i].nGrooveRight = m_strctFileRecs[i].m_nGrooveRight;
#else
		m_strctFileRecsBuffer[i].nGrooveLeft = 0;
		m_strctFileRecsBuffer[i].nGrooveRight = 0;
#endif

		m_strctFileRecsBuffer[i].nRunnerGauge = 0;
		m_strctFileRecsBuffer[i].nRunnerLeftGauge = 0;
		m_strctFileRecsBuffer[i].nRunnerRightGauge = 0;
	}

	try
	{
		nBytesToWrite = nNbOfValidMeasRecordsInBuffer * (UINT)sizeof(m_strctFileRecsBuffer[0]);
		m_oOutputFile.Write((LPVOID)&m_strctFileRecsBuffer, nBytesToWrite);
		m_nNbOfStoredMeasRecords += nNbOfValidMeasRecordsInBuffer;
	}
	catch(CException* pErr)
	{
	#ifdef _DEBUG
		pErr->ReportError();
	#endif
		pErr->Delete();
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja zapisuje bufor
// Parametry:
// nNbOfValidMeasRecordsInBuffer - liczba poprawnych punktow pomiarowych w buforze
BOOL CTEC2FileOperator :: TEC2_WriteToBuffer(int nNbOfValidMeasRecordsInBuffer)
{
	ASSERT(nNbOfValidMeasRecordsInBuffer > 0 && nNbOfValidMeasRecordsInBuffer < CONVERT_TEC2RECORDS_BUFFER_SIZE);
	try
	{
		/* zapamietujemy dane paczkami */
		for (int i = 0; i < nNbOfValidMeasRecordsInBuffer; i++)
			m_oaFileRecs.Add(m_strctFileRecs[i]);

		// elementy inspekcji sa w m_oaInspekcjaFileRecs - nie trzeba ich przepisywac ;)
		
		m_nNbOfStoredMeasRecords += nNbOfValidMeasRecordsInBuffer;
	}
	catch(CException* pErr)
	{
	#ifdef _DEBUG
		pErr->ReportError();
	#endif
		pErr->Delete();
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja zwraca domyslny (pusty) rekord pomiarowy
CTEC2_FILEBUFFERREC CTEC2FileOperator :: TEC2_GetDefaultDocFileMeasRec()
{
	CTEC2_FILEBUFFERREC strctFileRec;

	strctFileRec.m_width				= NODRAWFLAG;
	strctFileRec.m_cant				= NODRAWFLAG;
	strctFileRec.m_vertIrrL			= NODRAWFLAG;
	strctFileRec.m_horizIrrL			= NODRAWFLAG;
	strctFileRec.m_nGrooveLeft		= NODRAWFLAG;
	strctFileRec.m_nGrooveRight		= NODRAWFLAG;
	strctFileRec.m_widthSSP_R			= NODRAWFLAG;
	strctFileRec.m_widthSSP_L			= NODRAWFLAG;
	strctFileRec.m_heightSSP_R		= NODRAWFLAG;
	strctFileRec.m_heightSSP_L		= NODRAWFLAG;
	strctFileRec.m_nFlags				= 0x0;
	strctFileRec.m_dbevents			= 0x00;
	strctFileRec.m_nSectionID			= NODRAWFLAG;
	strctFileRec.m_events				= 0x00;
	strctFileRec.m_nfKm				= 0.0;

	return strctFileRec;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja pobiera z zadanego pliku wielkosc pakietu i kod
// Jesli udalo sie odczytac wielkosc pakiety zwracany jest TRUE. 
BOOL CTEC2FileOperator :: GetFilePacketCodeAndSize(CFile &oDataFile, int& nPacketSize, int& nPacketCode)
{
	struct
	{
		USHORT nPacketSize;
		BYTE   nPacketCode;
	} strctFilePacketHdr;
	
	UINT nReadBytes;
	try
	{
		/* jesli osiagnelismy koniec pliku */
		if (oDataFile.GetPosition() == oDataFile.GetLength())
		{
			return FALSE;
		}

		nReadBytes = oDataFile.Read((LPVOID)&strctFilePacketHdr, sizeof(strctFilePacketHdr));

		if (nReadBytes != sizeof(strctFilePacketHdr))
		{
			nPacketSize = nReadBytes;
			return FALSE;
		}
		nPacketSize = strctFilePacketHdr.nPacketSize;
		nPacketCode = (int)strctFilePacketHdr.nPacketCode;

		TRACE("Odczytano (*.teec): %d bajtów - kod: %X\n", nPacketSize, nPacketCode);
	}
	catch(CException* pErr)
	{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif

		pErr->Delete();

		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja sprawdza czy pobrany kod jest jednym z rozpoznawanych
// - w komentarz wrzucone zostaly znaczniki dla pliku rozjazdowego
// N oznacza ze dany kod zostal dodany w stosunku do DTG
BOOL CTEC2FileOperator :: ValidateFCodeFromFile(int nFCode)
{
	switch (nFCode)
	{
		case FCODE_SYGNATURA:			/* sygnatura pliku */
		case FCODE_NAGL_TOR:			/* naglowek pliku torowego */
		case FCODE_NOTATKA:				/* luŸna notatka dotycz¹ca pomiaru, mo¿e ich byæ kilka */
		case FCODE_KORKIL:				/* korekta kilometrazu */
		case FCODE_KORNAL:				/* zmiana sposobu naliczania */
		case FCODE_ZMIANALOK:			/* pakiet zmiany lokalizacji (linii, toru, szlaku) */
/*N*/	case FCODE_RAMKA_POM:			/* ramka pomiarowa  */
/*N*/	case FCODE_OPIS_RAMKI_POM:		/* opis ramki pomiarowej */
/*N*/	case FCODE_OPIS_ELEM_INSP:		/* opis elementu inspekcji (przegladu technicznego) */
/*N*/	case FCODE_ELEM_INSP:			/* wynikowy element inspekcji */
/*N*/	case FCODE_KASUJ_ELEM_INSP:		/* informacja o skasowaniu elementu inspekcji */
/*N*/	case FCODE_OPIS_ELEM_INSP_EX:	/* rozszerzony opis elementu inspekcji */
		/* te stale olewamy */
/*N*/	// case FCODE_OPISLINII:			/* opis linii */
/*N*/	// case FCODE_OPISODCINKA:			/* opis odcinka */
		// case FCODE_NAGL_ROZJ:
		// case FCODE_ZMIERZPUNCHAR:
		// case FCODE_RODZAJ_ROZJ:
		// case FCODE_NAGL_INSP_ROZJ:
		// case FCODE_TYP_ROZJ:
		// case FCODE_TYP_ZESTAW_PCH:
		// case FCODE_OPISPUNCHAR:
		// case FCODE_BIT_NEXT:
		// case FCODE_RYSUNEK_BIN:
		// case FCODE_DANE_ROZ_EX:
		// case FCODE_USTTEE_NOTATKI:
		// case FCODE_RAMPAPRZECH:
		// case FCODE_DEF_ZESTAW_KOL:
		// case FCODE_DEVICEDATA:			/* pakiet nastaw urzadzenia */
		// case FCODE_TEMPERATURA:			/* temperatura w momencie rozpoczecia pomiaru */
		// /* te stale nie wystapia juz w tej wersji pliku pomiarowego - tu bedzie opis ramki pomiarowej i potem ramka */
		// case FCODE_USTERKI:				/* pakiet opisujccy bity usterek - jak w starym TEC'u */
		// case FCODE_PRZECHNOM:			/* ustawienie wartosci przechylki nominalnej */
		// case FCODE_SZERPRZECH:			/* pakiet pomiaru szerokosci i przechylki */
		// case FCODE_SZERPRZECH_KROK:		/*  */
		// case FCODE_ROWEKL:				/* pakiet pomiaru zlobka lewego i szerokosci prowadzenia lewego */
		// case FCODE_ROWEKL_KROK:			/*  */
		// case FCODE_ROWEKP:				/* pakiet pomiaru zlobka prawego i szerokosci prowadzenia prawego */
		// case FCODE_ROWEKP_KROK:			/*  */
		// case FCODE_SZERPROW:			/* pakiet pomiaru szerokosci miedzy prowadzeniami */
		// case FCODE_SZERPROW_KROK:		/*  */
			break;
		default:
			return FALSE;
			break;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja pobiera naglowek na podstawie pakietu z sygnatura zadanego parametrem
// Parametry:
// pPacketBuffor - wskaznik na bufor zawierajacy dane pakietu
// nPacketSize - rozmiar pakietu w buforze
// nVersion - wersja pliku
BOOL CTEC2FileOperator :: GetMeasurementHeaderInformations(BYTE* pPacketBuffor, int nPacketSize, int nVersion)
{
	_TEC2_DATETIME		strctTEC2DateTime;
	int					nFldSize = 0;

	if (nPacketSize < 3)
		return FALSE;

	//////////////////////////////////////////////////////////////////////
	/* konwertujemy dane z bufora do postaci struktury naglowka pomiaru */
	//////////////////////////////////////////////////////////////////////
	/* data i czas */
	strctTEC2DateTime = *(_TEC2_DATETIME *)pPacketBuffor;
	/* pomijamy pakiet daty i czas */
	pPacketBuffor += sizeof(_TEC2_DATETIME);
	
	m_strctTEC2MeasHeader.oMeasDateTime.SetDateTime(strctTEC2DateTime.nYear, strctTEC2DateTime.nMonth, strctTEC2DateTime.nDay,
										 strctTEC2DateTime.nHour, strctTEC2DateTime.nMinute, strctTEC2DateTime.nSec);

#ifdef _DEBUG
	CString strDate;			
	strDate = m_strctTEC2MeasHeader.oMeasDateTime.Format(VAR_DATEVALUEONLY);
#endif

	/* identyfikator "drogi" */
	m_strctTEC2MeasHeader.strRouteID.Format("%s", pPacketBuffor);
	pPacketBuffor += m_strctTEC2MeasHeader.strRouteID.GetLength() + 1;

	/* nazwa "drogi" */
	m_strctTEC2MeasHeader.strRouteName.Format("%s", pPacketBuffor);
	pPacketBuffor += m_strctTEC2MeasHeader.strRouteName.GetLength() + 1;

	/* nazwa sekcji */
	m_strctTEC2MeasHeader.strSectionOrStationName.Format("%s", pPacketBuffor);
	pPacketBuffor += m_strctTEC2MeasHeader.strSectionOrStationName.GetLength() + 1;

	/* Track ID */
	m_strctTEC2MeasHeader.strTrackID.Format("%s", pPacketBuffor);
	pPacketBuffor += m_strctTEC2MeasHeader.strTrackID.GetLength() + 1;
	
	/* Counting type */
	m_strctTEC2MeasHeader.nCountingType = *(short *)pPacketBuffor;
	pPacketBuffor += sizeof(short);

	if (m_strctTEC2MeasHeader.nCountingType != ROAD_COUNTER_DISABLED)
	{
		/* kilometr poczatkowy */
		m_strctTEC2MeasHeader.nfBegKm = *(double *)pPacketBuffor;
		pPacketBuffor += sizeof(double);

		m_strctTEC2MeasHeader.nfBegKm /= 1000.; // ??? na metry ???
		m_strctTEC2MeasHeader.nfEndKm = m_strctTEC2MeasHeader.nfBegKm;

		/* krok pomiarowy */
		m_strctTEC2MeasHeader.nfMeasStep = *(double *)pPacketBuffor;
		m_strctTEC2MeasHeader.nfMeasStep /= 1000.;				// do weryfikacji TEC2
		pPacketBuffor += sizeof(double);

		/* kierunek */
		m_strctTEC2MeasHeader.bDirectionInc = (pPacketBuffor[0] == 0) ? TRUE : FALSE;
		pPacketBuffor += sizeof(m_strctTEC2MeasHeader.bDirectionInc);

		if (m_strctTEC2MeasHeader.nCountingType == SLEEPER_ROAD_COUNTER)
		{
			/* krok pomiarowy mierzony w podkladach */
			m_strctTEC2MeasHeader.nNbOfSleepersForStep = *(unsigned char *)pPacketBuffor;
			pPacketBuffor += sizeof(unsigned char);
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja przesuwa kursor odczytu pliku odebranego z TEC2 o zadana liczne bajtow 
// (przeskakujemy nieznane pakiety)
// Parametry:
// oInputFile - plik w ktorym nalezy przeskoczyc dane
// nNbOfBytesToSkip - liczba bytow do przeskoczenia
BOOL CTEC2FileOperator :: SkipPartOfFile(CFile &oInputFile, int nNbOfBytesToSkip)
{
	TRACE("Pomijamy fragment pliku %d\n", nNbOfBytesToSkip);

	try
	{
		oInputFile.Seek(nNbOfBytesToSkip, CFile :: current);
	}
	catch(CException* pErr)
	{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif
		pErr->Delete();
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja pobiera z zadanego pliku wielkosc pakietu i kod
BOOL CTEC2FileOperator :: ReadFilePacket(CFile &oDataFile, BYTE *pDataBuffor, int &nPacketSize)
{
	UINT nReadBytes;
	ASSERT(pDataBuffor != NULL && nPacketSize <= FILE_PACKET_BUFFOR_SIZE);

	try
	{
		ASSERT(oDataFile.GetPosition() < oDataFile.GetLength());

		nReadBytes = oDataFile.Read((LPVOID)pDataBuffor, nPacketSize);
		if (nReadBytes != nPacketSize)
		{
			nPacketSize = nReadBytes;
			return FALSE;
		}
	}
	catch(CException* pErr)
	{
		#ifdef _DEBUG
			pErr->ReportError();
		#endif
		pErr->Delete();
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja konwertuje odczytany pakiet do postaci wlasciwej danej w rekordzie pomiarowym
// Parametry:
// pFilePacketBuffor - wskznik na bufor zawierajacy dane pakietu
// nPacketFCode - kod pakietu w buforze
// nPacketSize - rozmiar pakietu w buforze
// bCounterDisabled - flaga wskazujaca czy zadany pomiar wykonany byl z wylaczonym licznikiem drogi
// nfCurrKm - kilometraz biezacego punktu pomiarowego
// nfCurrStep - krok pomiaru
// bCurrDirectionIncreasing - flaga okreslajaca czy pomiar jest rosnacy czy malejacy
// nNbOfValidMeasRecordsInBuffer - licznik odczytanych rekordow pomiarowych
BOOL CTEC2FileOperator :: TEC2_ConvertReceivedData(CGeoTecDoc* pDoc, BYTE* pFilePacketBuffor, int nPacketFCode, int nPacketSize,
												BOOL &bCounterDisabled,	double &nfCurrKm, double &nfCurrStep,
												BOOL &bCurrDirectionIncreasing, int &nNbOfValidMeasRecordsInBuffer)
{
	ASSERT(nNbOfValidMeasRecordsInBuffer >= 0 && nNbOfValidMeasRecordsInBuffer + 1 < CONVERT_TEC2RECORDS_BUFFER_SIZE);
	CString strData;
	int nCountingType, nIdentyfikatorDoUsuniecia = -1;
	BOOL bDirectionInc;

	CTEC2_INSPEKCJAREC inspekcjaTemp;

	switch(nPacketFCode)
	{
		///////////////////////////
		/* opis ramki pomiarowej */
		///////////////////////////
		case FCODE_OPIS_RAMKI_POM:
			/* pobieram dane z pakietu - liczbe parametrow i tablice opisujaca ramke pomiarowa */
			m_nLiczbaParametrow = *(short *)pFilePacketBuffor;
			pFilePacketBuffor += sizeof(short);

			for (int i = 0, nOffset = 0; i < m_nLiczbaParametrow; i++, nOffset += sizeof(short))
				m_nIDParametrow[i] = *(short *)(pFilePacketBuffor + nOffset);

			/* i tworze rekord na dane pomiarowe opisane przez m_nLiczbaParametrow */
			// m_pDataRecord = new int[m_nLiczbaParametrow];
			break;
		/////////////////////
		/* ramka pomiarowa */
		/////////////////////
		case FCODE_RAMKA_POM:
			// utworzenie odpowiedniej struktury danych, na podstawie otrzymanych parametrow (pod dane przychodzace w
			// nastepnych ramkach pomiarowych - np tablice floatow 
			// (uwaga na m_oaFileRecs - tu laduja docelowo po calkowitym odczycie)

			/* pobieram dane z pakietu - tablice parametrow (pomierzone parametry) */
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer] = TEC2_GetDefaultDocFileMeasRec();
			if (bCounterDisabled == TRUE)
			{
				m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nfKm = double(nNbOfValidMeasRecordsInBuffer);
				m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nSectionID = m_nSectionID;
			}

			m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_GAUGE)]] = FloatToInt(*(float *)pFilePacketBuffor);
			 pFilePacketBuffor += sizeof(float);
			m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_CANT)]] = FloatToInt(*(float *)pFilePacketBuffor);
			 pFilePacketBuffor += sizeof(float);
			m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_VERT)]] = FloatToInt(*(float *)pFilePacketBuffor);
			 pFilePacketBuffor += sizeof(float);
			m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_HORZ)]] = FloatToInt(*(float *)pFilePacketBuffor);

#ifdef TEC2WITHGROOVES
			 pFilePacketBuffor += sizeof(float);
			m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_GR_R)]] = FloatToInt(*(float *)pFilePacketBuffor);
			 pFilePacketBuffor += sizeof(float);
			m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_GR_L)]] = FloatToInt(*(float *)pFilePacketBuffor);
#endif

			/* zapamietuje dane w tablicy - na podstawie opisu */
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_width		 = m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_GAUGE)]];
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_cant		 = m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_CANT)]];
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_vertIrrL	 = m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_VERT)]];
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_horizIrrL   = m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_HORZ)]];
			

#ifdef TEC2WITHGROOVES
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nGrooveLeft	= m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_GR_L)]];
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nGrooveRight   = m_oaDataRecord[m_nIDParametrow[PobierzIDParametru(IDPAR_GR_R)]];
#endif

			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nfKm		 = nfCurrKm;
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_events		 = m_nCurrModalEventsMask;

			/* kilometraz dla nastepnego kroku pomiarowego */
			if (bCurrDirectionIncreasing)
				nfCurrKm += nfCurrStep;	
			if (!bCurrDirectionIncreasing)
				nfCurrKm -= nfCurrStep;	

			nNbOfValidMeasRecordsInBuffer++;
			break;
		///////////////////////
		// element inspekcji //
		///////////////////////
		case FCODE_ELEM_INSP:
			/* pobieramy typ_elementu */
			inspekcjaTemp.m_nTypElementu = *(short *)pFilePacketBuffor;
			pFilePacketBuffor += sizeof(short);

			/* pobieramy flagi */
			inspekcjaTemp.m_nFlagi = *(short *)pFilePacketBuffor;
			pFilePacketBuffor += sizeof(short);

			/* pobieramy identyfikator */
			inspekcjaTemp.m_nIdentyfikator = *(unsigned short *)pFilePacketBuffor;
			pFilePacketBuffor += sizeof(unsigned short);

			/* jesli nie jest zapalona flaga, to tekst elementu jest dodawany do pelnego tekstu notatki ??? :-/ ??? 
			   - cokolwiek to nie znaczy. Otrzymalem informacje, ze jesli jest zapalona, to nie interesuja mnie
			   flagi 0x8 i 0x10 i tekst w m_strTekst wrzucam do pola ELEMENT_NAME tabeli INSPECTIONS z bazy danych */
			if (!(inspekcjaTemp.m_nFlagi & 0x0001))
			{
				/* pobieramy tekst elementu */
				inspekcjaTemp.m_strTekst.Format("%s", pFilePacketBuffor);
				pFilePacketBuffor += inspekcjaTemp.m_strTekst.GetLength() + 1;
			}

			/* //-- zdarzenie punktowe --//
			   jesli we flagach ustawiono bit zdarzenia i nie jest to zdarzenie modalne - pobieramy bit zdarzenia
			   i przypisujemy go poprzedniej paczki danych */
			if ((inspekcjaTemp.m_nFlagi & 0x0008) && !(inspekcjaTemp.m_nFlagi & 0x0010))
			{
				/* sprawdzam, czy - w zaleznosci od kodu - sa jakies dane -
				   - w przypadku braku szczegolowych danych: */
				if (!SprawdzIDodajSzczegoloweDane(inspekcjaTemp, pFilePacketBuffor, nfCurrKm, nNbOfValidMeasRecordsInBuffer))
				{
					/* na podstawie identyfikatora z pliku - okreslam typ zdarzenia starego typu */
					if (DodajZdarzenieModalneDoMaski(OkreslBitZdarzenia(inspekcjaTemp.m_nIdentyfikator)))
						m_strctFileRecs[(nNbOfValidMeasRecordsInBuffer > 0) ? nNbOfValidMeasRecordsInBuffer - 1 : nNbOfValidMeasRecordsInBuffer].m_events = m_nCurrModalEventsMask;

					/* jest to zdarzenie punktowe, wiec od razu wywalamy go z maski - aby nie dopisalo sie do kolejnej paczki danych */
					UsunZdarzenieModalneZMaski(OkreslBitZdarzenia(inspekcjaTemp.m_nIdentyfikator));

					/* to jest zdarzenie wg starego typu - w tym wypadku - skoro dopisalismy zdarzenie do *.events, to nie wpisujemy go 
					   do elementu inspekcji (wyswietlamy w Visualu po staremu z mozliwoscia maskowania) */
					inspekcjaTemp.m_nIdentyfikator = 0;
				}
				/* w przypadku szczegolowych danych: np tekst - mam je w obiekcie inspekcjaTemp i dodaje do kolekcji */
				else 
				{
					inspekcjaTemp.m_nfKmBeg = inspekcjaTemp.m_nfKmEnd = nfCurrKm;
					inspekcjaTemp.m_nIdentyfikator = OkreslBitZdarzenia(inspekcjaTemp.m_nIdentyfikator);

					m_oaInspekcjaFileRecs.Add(inspekcjaTemp);

					/* zwiekszamy licznik inspekcji */
					m_nNbOfStoredInspectionRecords++;
				}

				break;
			}

			/* //-- zdarzenie modalne --//
			   jesli we flagach ustawiono bit zdarzenia i jest to zdarzenie modalne - pobieramy bit zdarzenia
			   i przypisujemy go do poprzedniej paczki danych - ustawiamy nZdazeniaModalne ktora dopisze do kazdej paczki 
			   o jeden rekord wczesniej - ze zmiennej bedzie kasowany bit zdarzenia po otrzymaniu pakietu FCODE_KASUJ_EL_INSP */
			if ((inspekcjaTemp.m_nFlagi & 0x0008) && (inspekcjaTemp.m_nFlagi & 0x0010))
			{
				/* sprawdzam, czy - w zaleznosci od kodu - sa jakies dane -
				   - w przypadku braku szczegolowych danych: */
                if (!SprawdzIDodajSzczegoloweDane(inspekcjaTemp, pFilePacketBuffor, nfCurrKm, nNbOfValidMeasRecordsInBuffer))
				{
					/* na podstawie identyfikatora z pliku - okreslam typ zdarzenia starego typu (UWAGA! modalnego) */
					if (DodajZdarzenieModalneDoMaski(OkreslBitZdarzenia(inspekcjaTemp.m_nIdentyfikator)))
						m_strctFileRecs[(nNbOfValidMeasRecordsInBuffer > 0) ? nNbOfValidMeasRecordsInBuffer - 1 : nNbOfValidMeasRecordsInBuffer].m_events = m_nCurrModalEventsMask;

					/* to jest zdarzenie wg starego typu - w tym wypadku - skoro dopisalismy zdarzenie do *.events, to nie wpisujemy go 
					   do elementu inspekcji (wyswietlamy w Visualu po staremu z mozliwoscia maskowania) */
					inspekcjaTemp.m_nIdentyfikator = 0;
				}
				/* w przypadku szczegolowych danych: np tekst - mam je w obiekcie inspekcjaTemp i dodaje do kolekcji */
				else 
				{
					inspekcjaTemp.m_nIdentyfikator = OkreslBitZdarzenia(inspekcjaTemp.m_nIdentyfikator);

					m_oaInspekcjaFileRecs.Add(inspekcjaTemp);
				}

				break;
			}

			//-- inspekcja punktowa --//
			/* jesli we flagach nie sa ustawione zadne bity - tj. bit ze jest to zdarzenie, ani bit ze jest to ciagle (zdarzenie/inspekcja) */
			if (!(inspekcjaTemp.m_nFlagi & 0x0008) && !(inspekcjaTemp.m_nFlagi & 0x0010))
			{
				/* zapisuje sobie kilometraz koncowy wystapienia inspekcji - skoro jest to inspekcja punktowa, to
				   kilometraz koncowy bedzie taki sam jak poczatkowy - w sumie moze sie to do czegos przydac pozniej */
				inspekcjaTemp.m_nfKmBeg = inspekcjaTemp.m_nfKmEnd = nfCurrKm;

				/* sprawdzam, czy - w zaleznosci od kodu - sa jakies dane - np tekst - i wrzucam przez referencje do obiektu inspekcjaTemp */
				SprawdzIDodajSzczegoloweDane(inspekcjaTemp, pFilePacketBuffor, nfCurrKm, nNbOfValidMeasRecordsInBuffer);

				m_oaInspekcjaFileRecs.Add(inspekcjaTemp);

				/* zwiekszamy licznik inspekcji */
				m_nNbOfStoredInspectionRecords++;
				break;
			} 

			//-- inspekcja modalna -- nie obslugiwana (w momencie pisania kodu - po stronie urzadzenia) //
			if (!(inspekcjaTemp.m_nFlagi & 0x0008) && (inspekcjaTemp.m_nFlagi & 0x0010))
			{
				/* zapisuje sobie kilometraz poczatkowy i koncowy wystapienia inspekcji */
				inspekcjaTemp.m_nfKmBeg = nfCurrKm;

				/* sprawdzam, czy - w zaleznosci od kodu - sa jakies dane - np tekst - i wrzucam przez referencje do obiektu inspekcjaTemp */
				SprawdzIDodajSzczegoloweDane(inspekcjaTemp, pFilePacketBuffor, nfCurrKm, nNbOfValidMeasRecordsInBuffer);

				m_oaInspekcjaFileRecs.Add(inspekcjaTemp);
		
				/* zwiekszamy licznik inspekcji */
				m_nNbOfStoredInspectionRecords++;
				break;
			}
			break;
		/////////////////////////////
		// kasuj element inspekcji //
		/////////////////////////////
		case FCODE_KASUJ_ELEM_INSP:
			/* zapisujemy identyfikator elementu do usuniecia */
			nIdentyfikatorDoUsuniecia = *(unsigned short *)pFilePacketBuffor;
			pFilePacketBuffor += sizeof(unsigned short);

			/* sprawdzam, czy jest to inspekcja, czy zwykle zdarzenie modalne, czy zwykle zdarzenie modalne z tekstem
			   (jesli to ostatnie, to zapisane w tablicy inspekcji) - 
			   jesli identyfikator nie zostal zapisany, to wiem, ze jest to zwykla usterka modalna "bez tekstu", 
			   a nie zwykla usterka modalna "z tekstem" lub inpekcja modalna (ta ostatnia i tak nie pojawi sie, bo nie ma 
			   jej implementacji w pulpicie) */
			if (WyszukajIdentyfikatorInspekcji(nIdentyfikatorDoUsuniecia) == -1) 
			{
				// if (m_nCurrModalEventsMask & nIdentyfikatorDoUsuniecia)
				if (m_nCurrModalEventsMask & OkreslBitZdarzenia(nIdentyfikatorDoUsuniecia))
				{
					UsunZdarzenieModalneZMaski(OkreslBitZdarzenia(nIdentyfikatorDoUsuniecia));
					m_strctFileRecs[nNbOfValidMeasRecordsInBuffer - 1].m_events = m_nCurrModalEventsMask;
				}
				else WylaczInspekcjeModalna(OkreslBitZdarzenia(nIdentyfikatorDoUsuniecia), nfCurrKm);
			}
			else 
			{
				/* wylaczam zdarzenie starego typu traktowane jako inspekcja (okreslam jego maske na podstawie id i dopisuje kilometraz koncowy */
				if (WylaczInspekcjeModalna(nIdentyfikatorDoUsuniecia, nfCurrKm))
					m_nNbOfStoredInspectionRecords++;
			}

			nIdentyfikatorDoUsuniecia = -1;
			break;
		///////////////////////////
		/* notatka - standardowa */
		///////////////////////////
		case FCODE_NOTATKA:
			/* zapisujemy do stringa tresc notatki - na razie nic z tym nie robimy 
			   (trzeba pomyslec w jaki sposob zapisac to do bazy, aby sensownie pobrac w Visualu */
			strData.Format("%s", pFilePacketBuffor);
			pFilePacketBuffor += strData.GetLength() + 1;

			m_nCurrModalEventsMask |= TEC2_NOTE_EVENT_MASK;

			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer - 1].m_events = m_nCurrModalEventsMask;

			m_nCurrModalEventsMask &= ~(TEC2_NOTE_EVENT_MASK);
		
			// if (StoreEventValue(pDoc, nNbOfValidMeasRecordsInBuffer, TEC2_NOTE_EVENT_MASK, nfCurrKm, m_nSectionID, strData) == FALSE)
			//	return FALSE;

			break;
		////////////////////////////////////
		/* korekta kilometrazu - wartosci */
		////////////////////////////////////
		case FCODE_KORKIL:
			TRACE("Korekta @ %.4f -> ", nfCurrKm);
			nfCurrKm = *(double *)pFilePacketBuffor;
			nfCurrKm /= 1000;
			TRACE("%.4f\n", nfCurrKm);
			pFilePacketBuffor += sizeof(double);
			// nNbOfValidMeasRecordsInBuffer++;
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer] = TEC2_GetDefaultDocFileMeasRec();
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_events = m_strctFileRecs[nNbOfValidMeasRecordsInBuffer - 1].m_events & m_POINT_EVENTS_MASK_NEG;
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nfKm = nfCurrKm;
			m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nSectionID = m_nSectionID; 

			/* sprawdzamy kierunek narastania kilometrazu (aby moc usunac niepotrzebne dane - bo cofnelismy sie wozkiem) */
			if (m_strctTEC2MeasHeader.bDirectionInc)
			{
				if (nfCurrKm < m_strctFileRecs[nNbOfValidMeasRecordsInBuffer - 1].m_nfKm)
					SkorygujPomiarPrzezCofanie(nfCurrKm, nNbOfValidMeasRecordsInBuffer);
			}
			if (!m_strctTEC2MeasHeader.bDirectionInc)
			{
				if (nfCurrKm > m_strctFileRecs[nNbOfValidMeasRecordsInBuffer - 1].m_nfKm)
					SkorygujPomiarPrzezCofanie(nfCurrKm, nNbOfValidMeasRecordsInBuffer);
			}			
			
			break;
		////////////////////////////////////////////////////////////
		/* korekta naliczania kilometrazu - tj. kroku pomiarowego */
		////////////////////////////////////////////////////////////
		case FCODE_KORNAL:
			nCountingType = *(short *)pFilePacketBuffor;
			pFilePacketBuffor += sizeof(short);

			bDirectionInc = FALSE;
			/* jesli typ naliczania kilometrazu jest inny niz "brak" */
			if (nCountingType != 0)
			{
				m_strctTEC2MeasHeader.nfBegKm = *(double *)pFilePacketBuffor;
				 pFilePacketBuffor += sizeof(double);
				nfCurrStep = *(double *)pFilePacketBuffor;
				 pFilePacketBuffor += sizeof(double);
				bDirectionInc = (*(unsigned char *)pFilePacketBuffor) == 0 ? TRUE : FALSE;
				if (nCountingType == ROAD_COUNTER_DISABLED)
				{
					m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nFlags |= ROAD_COUNTER_DISABLED_FLAG;
					m_nSectionID++;
					bCounterDisabled = TRUE;
				}
				if (nCountingType == SLEEPER_ROAD_COUNTER)
				{
					/* krok pomiarowy mierzony w podkladach */
					m_strctTEC2MeasHeader.nNbOfSleepersForStep = *(unsigned char *)pFilePacketBuffor;
					pFilePacketBuffor += sizeof(unsigned char);
					bCurrDirectionIncreasing = nCountingType;
					m_strctFileRecs[nNbOfValidMeasRecordsInBuffer].m_nFlags |= SLEEPER_ROAD_COUNTER_FLAG;
					m_nSectionID++;
					if (!bCurrDirectionIncreasing)
						nfCurrStep *= (-1);
					bCounterDisabled = FALSE;
				}
			}
			break;
		////////////////////////////////
		/* zmiana lokalizacji pomiaru */
		////////////////////////////////
		case FCODE_ZMIANALOK:
			/* numer linii */
			m_strctTEC2MeasHeader.strRouteID.Format("%s", pFilePacketBuffor);
			pFilePacketBuffor += m_strctTEC2MeasHeader.strRouteID.GetLength() + 1;

			/* nazwa linii */
			m_strctTEC2MeasHeader.strRouteName.Format("%s", pFilePacketBuffor);
			pFilePacketBuffor += m_strctTEC2MeasHeader.strRouteName.GetLength() + 1;

			/* nazwa szlaku/stacji */
			m_strctTEC2MeasHeader.strSectionOrStationName.Format("%s", pFilePacketBuffor);
			pFilePacketBuffor += m_strctTEC2MeasHeader.strSectionOrStationName.GetLength() + 1;
			
			/* numer toru */
			m_strctTEC2MeasHeader.strTrackID.Format("%s", pFilePacketBuffor);
			pFilePacketBuffor += m_strctTEC2MeasHeader.strTrackID.GetLength() + 1;

			m_nSectionID++;
			break;
		//////////////////////////////////////////
		/* gdyby przyszlo cos, czego nie chcemy */
		//////////////////////////////////////////
		default:
			break;
	}
	m_nLastFCode = nPacketFCode;

	return TRUE;
}

BOOL CTEC2FileOperator :: SkorygujPomiarPrzezCofanie(double nfKmOdKtoregoZaczacNaNowo, int &nIloscDanychWBuforze)
{
	CTEC2_INSPEKCJAREC inspekcjaTemp;
	int nLicznikDoUsuniecia = 0;

	for (int i = 0; i < nIloscDanychWBuforze; i++)
	{
		if (AlmostEqualDoubles(m_strctFileRecs[i].m_nfKm, nfKmOdKtoregoZaczacNaNowo, 0.00001))
		{
			for (int j = i; j < nIloscDanychWBuforze; j++)
			{
				m_strctFileRecs[j] = TEC2_GetDefaultDocFileMeasRec();
				nLicznikDoUsuniecia++;
			}
			break;
		}
	}

	nIloscDanychWBuforze -= nLicznikDoUsuniecia; 

	m_strctFileRecs[nIloscDanychWBuforze].m_events = m_strctFileRecs[nIloscDanychWBuforze - 1].m_events & m_POINT_EVENTS_MASK_NEG;
	m_strctFileRecs[nIloscDanychWBuforze].m_nfKm = nfKmOdKtoregoZaczacNaNowo;
	m_strctFileRecs[nIloscDanychWBuforze].m_nSectionID = m_nSectionID; 

	/* inspekcje tez usuwamy - o ile wystapily */
	for (int k = 0; k < m_oaInspekcjaFileRecs.GetSize(); k++)
	{
		inspekcjaTemp = m_oaInspekcjaFileRecs.GetAt(k);
		if (AlmostEqualDoubles(inspekcjaTemp.m_nfKmBeg, nfKmOdKtoregoZaczacNaNowo, 0.00001) || 
			(inspekcjaTemp.m_nfKmBeg >= nfKmOdKtoregoZaczacNaNowo && m_strctTEC2MeasHeader.bDirectionInc) ||
			(inspekcjaTemp.m_nfKmBeg <= nfKmOdKtoregoZaczacNaNowo && !m_strctTEC2MeasHeader.bDirectionInc))
		{
			m_oaInspekcjaFileRecs.RemoveAt(k);
			m_nNbOfStoredInspectionRecords--;
		}
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// Funkcja porownuje wartosci double z precyzja nfEpsilon
// Parametry:
// double nfVal1 - wartosc pierwsza (do porownania)
// double nfVal2 - wartosc druga (do porownania)
// double nfEpsilon - precyzja, z jaka bedziemy porownywac
bool CTEC2FileOperator :: AlmostEqualDoubles(double nfVal1, double nfVal2, double nfEpsilon)
{
	bool bRet = (((nfVal2 - nfEpsilon) < nfVal1) && (nfVal1 < (nfVal2 + nfEpsilon)));
	return bRet;
}

BOOL CTEC2FileOperator :: SprawdzIDodajSzczegoloweDane(CTEC2_INSPEKCJAREC &inspekcjaTemp, BYTE* pFilePacketBuffor, double nfCurrKm, int nNbOfValidMeasRecordsInBuffer)
{
	BOOL bFoundAdditionalInfo = FALSE; 

	switch (inspekcjaTemp.m_nTypElementu)
	{
	/* jesli typ elementu to data */
	case TEI_DATE:
		{
		 inspekcjaTemp.m_dataInspekcji = *(strctData *)pFilePacketBuffor;
		  pFilePacketBuffor += sizeof(strctData);
		 bFoundAdditionalInfo = TRUE;
		break;
		}
	/* jesli typ elementu to notatka */
	case TEI_NOTATKA:
		{
		 inspekcjaTemp.m_strTekstNotatki.Format("%s", pFilePacketBuffor);
		  pFilePacketBuffor += inspekcjaTemp.m_strTekstNotatki.GetLength() + 1;
 		 bFoundAdditionalInfo = TRUE;
		break;
		}
	/* jesli typ elementu to liczba sztuk */
	case TEI_INT:
		{
		 inspekcjaTemp.m_nLiczbaSztuk = *(unsigned short *)pFilePacketBuffor;
		  pFilePacketBuffor += sizeof(unsigned short);
 		 bFoundAdditionalInfo = TRUE;
		break;
		}
	/* jesli typ elementu to liczba elementow zbioru */
	case TEI_NBSET:
		{
		 inspekcjaTemp.m_nLiczbaElementow = *(unsigned short *)pFilePacketBuffor;
		  pFilePacketBuffor += sizeof(unsigned short);
		 inspekcjaTemp.m_pTabElementy = new int[inspekcjaTemp.m_nLiczbaElementow];

		 for (int i = 0, nOffset = 0; i < inspekcjaTemp.m_nLiczbaElementow; i++, nOffset += sizeof(unsigned short))
			inspekcjaTemp.m_pTabElementy[i] = *(unsigned short *)(pFilePacketBuffor + nOffset);

 		 bFoundAdditionalInfo = TRUE;
		 break;
		}
	/* jesli typ elementu to wartosc double */
	case TEI_DOUBLE:
		{
		 inspekcjaTemp.m_nfWartosc = *(double *)pFilePacketBuffor;
		  pFilePacketBuffor += sizeof(double);		
 		 bFoundAdditionalInfo = TRUE;
		 break;
		}
	default: 
		break;
	}

	/* zapisuje sobie kilometraz poczatkowy wystapienia inspekcji */
	inspekcjaTemp.m_nfKmBeg = nfCurrKm;
	/* zapisuje sobie id paczki pomiarowej dla ktorej pojawila sie inspekcja */
	inspekcjaTemp.m_nIDPaczkiPomiarowej = (nNbOfValidMeasRecordsInBuffer > 0) ? nNbOfValidMeasRecordsInBuffer - 1 : nNbOfValidMeasRecordsInBuffer;

	if (bFoundAdditionalInfo)
		return TRUE;
	else return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja wyszukuje w tablicy parametrow (opisanych w pliku) okreslone ID
// i zwraca je
int CTEC2FileOperator :: PobierzIDParametru(int nParametr)
{
	for (int i = 0; i < m_nLiczbaParametrow && i < MAX_NUMBER_OF_PARAMS_FROM_FILE; i++)
		if (nParametr == m_nIDParametrow[i])
			return i;

	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja przelicza wartosci float na inty (poprzez wymnozenie - w ten sposob
// probujemy zapisac dane w bazie w postaci "wspolnej" dla roznych urzadzen
int CTEC2FileOperator :: FloatToInt(float nfFloatVal) 
{
	int nIntVal = -1;

	if (nfFloatVal >= 0.0)
		nIntVal = (int)(nfFloatVal * STORINGSCALE + 0.5);
	else nIntVal = (int)(nfFloatVal * STORINGSCALE - 0.5);

	return nIntVal;
}

///////////////////////////////////////////////////////////////////
// Funkcja zwraca maske zdarzen punktowych
int CTEC2FileOperator :: GetMaskOfPoinsEvents()
{
	int nMask = 0;

	nMask |= PHIZMARK;
	nMask |= EVP_BRAK_SRUB;
	nMask |= EVP_PEKNIETA_SZYNA;
	nMask |= EVP_PODKLAD;	
	nMask |= EVP_WYBUKSOWANIA;
	nMask |= EVP_PEKNIETA_SPOINA;
	nMask |= EVP_STYK;	
	nMask |= EVP_UBYTEK_SZYNY;

	return nMask;
}

///////////////////////////////////////////////////////////////////
// Funkcja 'wylicza' - na podstawie identyfikatora, bit zdarzenia
// wg starego typu (dane wejsciowe to identyfikator zdarzenia zawarty
// w ramce inspekcji - jako parametr wyjsciowy jest podawany bit
// zdarzenia starego typu - patrz zdarzenia.xls
int CTEC2FileOperator :: OkreslBitZdarzenia(int nIdentyfikator)
{
	int nBitZdarzenia = 0;

	/* sprawdzam, czy zdarzenie jest zdarzeniem starego typu */
	if (nIdentyfikator >= 0xFF0)
	{
        nBitZdarzenia = (1 << (0xFFFF - nIdentyfikator));
		return nBitZdarzenia;
	}

	return nIdentyfikator;
}

///////////////////////////////////////////////////////////////////
// Funkcja dodaje (wlacza) do maski aktualnych zdarzen bit okreslonego
// zdarzenia. Jako parametr wejsciowy jest podawany numer bitu do ustawienia
BOOL CTEC2FileOperator :: DodajZdarzenieModalneDoMaski(int nBitDoUstawienia)
{
	m_nCurrModalEventsMask |= nBitDoUstawienia;

	return TRUE;
}

///////////////////////////////////////////////////////////////////
// Funkcja usuwa (wylacza) z maski aktualnych zdarzen bit okreslonego 
// zdarzenia. Jako parametr wejsciowy jest podawany numer bitu do wygaszenia
BOOL CTEC2FileOperator :: UsunZdarzenieModalneZMaski(int nBitDoWygaszenia)
{
	m_nCurrModalEventsMask &= ~(nBitDoWygaszenia);

	return TRUE;
}

///////////////////////////////////////////////////////////////////
// Funkcja wyszukuje w CArray przeznaczonym na elementy inspekcji
// identyfikator inspekcji - jesli znajdzie - zwraca indeks w CArray,
// pod ktorym znajduje sie ten element, jesli nie znajdzie - zwraca -1
// oznacza to ze dany identyfikator to byl identyfikator zdarzenia
// modalnego, a nie inspekcji modalnej
int CTEC2FileOperator :: WyszukajIdentyfikatorInspekcji(int nIdentyfikator)
{
	CTEC2_INSPEKCJAREC tempInsp;

	for (int i = 0; i < m_oaInspekcjaFileRecs.GetSize(); i++)
	{
        tempInsp = m_oaInspekcjaFileRecs.GetAt(i);
		if (tempInsp.m_nIdentyfikator == nIdentyfikator)
			return i;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////
// Funkcja wyszukuje w CArray przeznaczonym na elementy inspekcji
// elementu opisanego identyfikatorem - jesli znajdzie - dopisuje
// kilometraz koncowy tej inspekcji i podmienia rekordy w CArray
BOOL CTEC2FileOperator :: WylaczInspekcjeModalna(int nIdentyfikatorUsterki, const double nfKilometrazKoncowyInspekcji)
{
	CTEC2_INSPEKCJAREC tempInsp;

	for (int i = m_oaInspekcjaFileRecs.GetSize() - 1; i >= 0 ; i--)
	{
        tempInsp = m_oaInspekcjaFileRecs.GetAt(i);
		if (tempInsp.m_nIdentyfikator == nIdentyfikatorUsterki)
		{
			tempInsp.m_nfKmEnd = nfKilometrazKoncowyInspekcji;
			m_oaInspekcjaFileRecs.SetAt(i, tempInsp);
			return TRUE;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Funkcja zapisuje notatke dopisana do zdarzenia 
// Parametry:
// nEvMask - maskaa zdarzenia, do ktorego dopisywane sa notatki
// nfEvKm - kilometraz w ktorym dopisano notatke
// nSection - numer odcinka, na ktorym dopisano notatke
// strEvAddData - notatka
BOOL CTEC2FileOperator :: StoreEventValue(CGeoTecDoc* pDoc, int nBufferIdx, int nEvMask, double nfEvKm, int nSection, CString strEvAddData)
{
	ASSERT(pDoc != NULL);
	ASSERT(nBufferIdx >= 0 && nBufferIdx < CONVERT_TEC2RECORDS_BUFFER_SIZE);
	int rv;
	int nEvIdx;

	nEvIdx = pDoc->m_oEvDecoder.GetMeasEventIdx(nEvMask);
	ASSERT(nEvIdx >= 0 && nEvIdx < NB_OF_EVENTS);
	if (nEvIdx < 0)
		return FALSE;
	rv = m_oaEventWriter[nEvIdx].WriteObjectLocalizationToFile(nSection, nfEvKm, nfEvKm, 0, strEvAddData, _T(""), 0);
	if (rv != RGOK)
		return FALSE;

	return TRUE;
}
