/***************************************************************
 * Name:      td5mapTable.cpp
 * Purpose:   Defines Application Frame
 * Author:    Luca Veronesi (luca72@libero.it)
 * Created:   2011-01-12
 * Copyright: Luca Veronesi ()
 * License:
 **************************************************************/


#include "td5mapeditorApp.h"
#include "td5mapTable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

td5mapTable::td5mapTable()
{
	m_index = 0;
	m_address = 0;
	m_cols = 0;
	m_rows = 0;
	m_map3d = FALSE;
	m_recognized = FALSE;
	m_type = 0;
	m_name = wxT("");
	m_comment = wxT("");
	m_singlevalue = false;
}

td5mapTable::~td5mapTable()
{

}

bool td5mapTable::ReadTable(wxWord* pwMapFileData, int iIndex, wxWord* pwBaseMapFileData)
{
    wxWord wIndex = (FUEL_MAP_BEGIN_ADDRESS + m_address) / sizeof(wxWord);

    if(!m_singlevalue)
    {
        pwMapFileData += wIndex;
        if(pwBaseMapFileData != NULL)
            pwBaseMapFileData += wIndex;
        m_cols = LoHi2HiLo(*pwMapFileData);

        pwMapFileData += 1;
        if(pwBaseMapFileData != NULL)
            pwBaseMapFileData += 1;
        m_rows = LoHi2HiLo(*pwMapFileData);
    }
    else
    {
        pwMapFileData += wIndex;
        if(pwBaseMapFileData != NULL)
            pwBaseMapFileData += wIndex;
        m_cols = 1;
        m_rows = 1;
    }

    m_headerCol.resize(m_cols);
    m_headerRow.resize(m_rows);
    m_tableData.resize(m_cols, m_rows);

    if (m_rows > 1)
        m_map3d= TRUE;

    if ((m_cols < 1) || (m_rows < 1) || (m_cols > 32) || (m_rows > 32))
        return FALSE;


    if (m_cols > 1)
        for(int c= 0; c < m_cols; c++)
        {
            pwMapFileData += 1;
            if(pwBaseMapFileData != NULL)
                pwBaseMapFileData += 1;
            m_headerCol[c].current = LoHi2HiLo(*pwMapFileData);
            if(pwBaseMapFileData != NULL)
                m_headerCol[c].base = LoHi2HiLo(*pwBaseMapFileData);
            else
                m_headerCol[c].base = 0;
        }

    if (m_rows > 1)
        for(int r= 0; r < m_rows; r++)
        {
            pwMapFileData += 1;
            if(pwBaseMapFileData != NULL)
                pwBaseMapFileData += 1;
            m_headerRow[r].current = LoHi2HiLo(*pwMapFileData);
            if(pwBaseMapFileData != NULL)
                m_headerRow[r].base = LoHi2HiLo(*pwBaseMapFileData);
            else
                m_headerRow[r].base = 0;
        }

    if ((m_cols > 1) || (m_rows > 1))
        for(int r = 0; r < m_rows; r++)
            for(int c = 0; c < m_cols; c++)
        {
            pwMapFileData += 1;
            if(pwBaseMapFileData != NULL)
                pwBaseMapFileData += 1;
            m_tableData[c][r].current = LoHi2HiLo(*pwMapFileData);
            if(pwBaseMapFileData != NULL)
                m_tableData[c][r].base = LoHi2HiLo(*pwBaseMapFileData);
            else
                m_tableData[c][r].base = 0;
        }

    if ((m_cols == 1) && (m_rows == 1))
    {
        m_headerCol[0].current = 0;
        m_headerRow[0].current = 0;
        m_tableData[0][0].current = LoHi2HiLo(*pwMapFileData);
        if(pwBaseMapFileData != NULL)
            m_tableData[0][0].base = LoHi2HiLo(*pwBaseMapFileData);
        else
            m_tableData[0][0].base = 0;
    }

    td5mapTableInfo mapinfo(m_mapID);
    m_recognized = mapinfo.GetInfoFromIndex(m_tableInfo, iIndex);
    m_type = m_tableInfo.m_type;
    m_name = m_tableInfo.m_name;
    m_comment = m_tableInfo.m_comment;

	return TRUE;
}

bool td5mapTable::WriteTable(wxWord* pwMapFileData)
{
	wxWord wIndex = (FUEL_MAP_BEGIN_ADDRESS + m_address) / sizeof(wxWord);

    if(!m_singlevalue)
    {
        pwMapFileData = pwMapFileData + wIndex + 1;
    }
    else
    {
        pwMapFileData = pwMapFileData + wIndex;
    }

	if (m_cols > 1)
		for(int c= 0; c < m_cols; c++)
		{
			pwMapFileData += 1;
			*pwMapFileData = HiLo2LoHi(m_headerCol[c].current/* + m_headerCol[c].diff*/);
		}

	if (m_rows > 1)
		for(int r= 0; r < m_rows; r++)
		{
			pwMapFileData += 1;
			*pwMapFileData = HiLo2LoHi(m_headerRow[r].current/* + m_headerRow[r].diff*/);
		}

	if ((m_cols > 1) || (m_rows > 1))
		for(int r = 0; r < m_rows; r++)
			for(int c = 0; c < m_cols; c++)
		{
			pwMapFileData += 1;
			*pwMapFileData = HiLo2LoHi(m_tableData[c][r].current/* + m_tableData[c][r].diff*/);
		}

    if ((m_cols == 1) && (m_rows == 1))
    {
        *pwMapFileData = HiLo2LoHi(m_tableData[0][0].current);
    }

	return TRUE;
}

void td5mapTable::EvalRange(int& min, int& max)
{
	max = 0;
	min = 0;

	for(int c = 0; c < m_cols; c++)
		for(int r = 0; r < m_rows; r++)
	{
		short data = m_tableData[c][r].current;

		if (data > max)
			max = data;
		if (data < min)
			min = data;
	}

	if (max == 0 )
		max = 1;
	else if (max < 10 )
		max = 10;
	else if (max < 100 )
		max = ((max / 10) + 1) * 10;
	else if (max < 1000 )
		max = ((max / 100) + 1) * 100;
	else if (max < 10000 )
		max = ((max / 1000) + 1) * 1000;
	else if (max < 100000 )
		max = ((max / 10000) + 1) * 10000;

	if (min == 0 )
		min = 0;
	else if (abs(min) < 10 )
		min = -10;
	else if (abs(min) < 100 )
		min = ((min / 10) - 1) * 10;
	else if (abs(min) < 1000 )
		min = ((min / 100) - 1) * 100;
	else if (abs(min) < 10000 )
		min = ((min / 1000) - 1) * 1000;
	else if (abs(min) < 100000 )
		min = ((min / 10000) - 1) * 10000;

	if (abs(min) > abs(max))
		max = abs(min);

	if ((abs(min) < abs(max)) && m_map3d)
		min = -max;
}

void td5mapTable::EvalDiffRange(int& min, int& max)
{
	max = 0;
	min = 0;

	for(int c = 0; c < m_cols; c++)
		for(int r = 0; r < m_rows; r++)
	{
		short data = m_tableData[c][r].current - m_tableData[c][r].base;

		if (data > max)
			max = data;
		if (data < min)
			min = data;
	}

	if (max == 0 )
		max = 1;
	else if (max < 10 )
		max = 10;
	else if (max < 100 )
		max = ((max / 10) + 1) * 10;
	else if (max < 1000 )
		max = ((max / 100) + 1) * 100;
	else if (max < 10000 )
		max = ((max / 1000) + 1) * 1000;
	else if (max < 100000 )
		max = ((max / 10000) + 1) * 10000;

	if (min == 0 )
		min = 0;
	else if (abs(min) < 10 )
		min = -10;
	else if (abs(min) < 100 )
		min = ((min / 10) - 1) * 10;
	else if (abs(min) < 1000 )
		min = ((min / 100) - 1) * 100;
	else if (abs(min) < 10000 )
		min = ((min / 1000) - 1) * 1000;
	else if (abs(min) < 100000 )
		min = ((min / 10000) - 1) * 10000;

	if (abs(min) > abs(max))
		max = abs(min);

	if ((abs(min) < abs(max)) && m_map3d)
		min = -max;
}

void td5mapTable::SumPercentCurrentValue(double sumpercent, int col, int row)
{
    short result = (m_tableData[col][row].current + (int)((double)m_tableData[col][row].current * sumpercent) / 100.0 );

    if ((result < 32767) && (result > -32768))
        m_tableData[col][row].current = result;
    else if (result > 32767)
        m_tableData[col][row].current = 32767;
    else if (result < -32768)
        m_tableData[col][row].current = -32768;
}

void td5mapTable::SumCurrentValue(int sum, int col, int row)
{
    short result = m_tableData[col][row].current + sum;

    if ((result < 32767) && (result > -32768))
        m_tableData[col][row].current = result;
    else if (result > 32767)
        m_tableData[col][row].current = 32767;
    else if (result < -32768)
        m_tableData[col][row].current = -32768;
}

bool td5mapTable::IsDifferentFromOriginal()
{
    bool diff = false;

	if ((m_cols > 1) || (m_rows > 1))
    {
        if (m_cols > 1)
            for(int c= 0; c < m_cols; c++)
            {
                if (m_headerCol[c].current != m_headerCol[c].base)
                    diff = true;
            }

        if (m_rows > 1)
            for(int r= 0; r < m_rows; r++)
            {
                if (m_headerRow[r].current != m_headerRow[r].base)
                    diff = true;
            }
		for(int r = 0; r < m_rows; r++)
			for(int c = 0; c < m_cols; c++)
            {
                if (m_tableData[c][r].current != m_tableData[c][r].base)
                    diff = true;
            }
    }

    if ((m_cols == 1) && (m_rows == 1))
    {
        if (m_tableData[0][0].current != m_tableData[0][0].base)
            diff = true;
    }

    return diff;
}
