package org.apache.jsp;

import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.jsp.*;
import java.io.*;
import java.util.Hashtable;
import java.util.Properties;
import java.util.Enumeration;
import cts.CassetteDB;
import cts.CassetteDBFactory;
import cts.CassetteIO;
import sil.beans.SilUtil;
import sil.beans.SilConfig;
import sil.beans.SilLogger;
import sil.servlets.ServletUtil;
import edu.stanford.slac.ssrl.authentication.utility.AuthGatewayBean;

public final class uploadForm_jsp extends org.apache.jasper.runtime.HttpJspBase
    implements org.apache.jasper.runtime.JspSourceDependent {


//============================================================
// global variables (declaration)
HttpSession g_session;
AuthGatewayBean g_gate;
CassetteDB ctsdb;



//==============================================================
//==============================================================
String getConfigValue(String paramName)
{
	String paramValue = SilConfig.getInstance().getProperty(paramName);
	if (paramValue==null) {
		String errmsg= "ERROR config.jsp getConfigValue() unknown property "+ paramName;
		errMsg(errmsg);
		return errmsg;
	}

	return paramValue;
}



//==============================================================
boolean checkAccessID(HttpServletRequest myrequest, HttpServletResponse myresponse)
    throws IOException
{
	boolean validSession = ServletUtil.checkAccessID(myrequest, myresponse);
	
	AuthGatewayBean gate = (AuthGatewayBean)myrequest.getSession().getAttribute("gate");
		
	if (!validSession)
		myresponse.sendRedirect("loginForm.jsp");
		
	if ((gate != null) && (g_gate != gate))
		g_gate = gate;
		
	return validSession;
}

//==============================================================
void errMsg(String msg)
{
    SilLogger.error( msg);
}

//==============================================================
void errMsg(Throwable ex)
{
    SilLogger.error(ex.toString());
    SilLogger.error(ex);
}


//==============================================================
void errMsg(String msg, Throwable ex)
{
    SilLogger.error(msg + ex.toString());
    SilLogger.error(ex);
}

//==============================================================
void logMsg( String msg)
{
    SilLogger.info(msg);
}


  private static java.util.List _jspx_dependants;

  static {
    _jspx_dependants = new java.util.ArrayList(2);
    _jspx_dependants.add("/config.jsp");
    _jspx_dependants.add("/pageheader.jsp");
  }

  public Object getDependants() {
    return _jspx_dependants;
  }

  public void _jspService(HttpServletRequest request, HttpServletResponse response)
        throws java.io.IOException, ServletException {

    JspFactory _jspxFactory = null;
    PageContext pageContext = null;
    HttpSession session = null;
    ServletContext application = null;
    ServletConfig config = null;
    JspWriter out = null;
    Object page = this;
    JspWriter _jspx_out = null;
    PageContext _jspx_page_context = null;


    try {
      _jspxFactory = JspFactory.getDefaultFactory();
      response.setContentType("text/html");
      pageContext = _jspxFactory.getPageContext(this, request, response,
      			null, true, 8192, true);
      _jspx_page_context = pageContext;
      application = pageContext.getServletContext();
      config = pageContext.getServletConfig();
      session = pageContext.getSession();
      out = pageContext.getOut();
      _jspx_out = out;


// uploadForm.jsp
//
// called by the Web page CassetteInfo.jsp
// create HTML form for the upload of an Excel file with Cassette information
// calls upload.jsp to handle the file upload
//

      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");

// config.jsp
// define parameters and JavaBeans for Crystal Cassette Tracking System
// provides basic functions to check the user access rights
//
// this file will be included by other JSP's with: %@include file="config.jsp"
//
// use JavaBean CassetteDB.class (WEB-INF/lib/cts.jar)
// to load XML from Oracle DB (with Oracle JDBC driver in WEB-INF/lib/classes111.jar)
//
// use JavaBean CassetteIO.class (WEB-INF/lib/cts.jar)
// for file-io, http transfer and xslt transformations
//
// use Java class AuthGatewayBean (WEB-INF/lib/authUtility.jar) to check user access rights
//  page import="edu.stanford.slac.ssrl.authentication.utility.AuthGatewayBean"
//
// define system configuration parameters like:
//   db connection parameters
//   directory path for file archive
//   beamline names
//   ...
// Most of the configuration parameters are loaded from the property file config.prop
// However some of them are hard coded in config.jsp:
//   db connection (DSN, userName, password)
//   normalized beamline names ( getBeamlineName())
//
// If you make a change in config.prop you should touch all jsp files
// touch *.jsp
// to make sure that Tomcat reloads all pages with the updated configuration
//
// If you add a beamline, you have to make changes in 3 places:
//   the db table beamline
//   the normalized beamline names in the function getBeamlineName() in config.jsp
//   create a new version of the xml-file beamlineList.xml with the help of getBeamlineList.jsp
//

      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      cts.CassetteIO ctsio = null;
      synchronized (session) {
        ctsio = (cts.CassetteIO) _jspx_page_context.getAttribute("ctsio", PageContext.SESSION_SCOPE);
        if (ctsio == null){
          ctsio = new cts.CassetteIO();
          _jspx_page_context.setAttribute("ctsio", ctsio, PageContext.SESSION_SCOPE);
        }
      }
      out.write("\r\n");
      out.write("\r\n");
      edu.stanford.slac.ssrl.authentication.utility.AuthGatewayBean gate = null;
      synchronized (session) {
        gate = (edu.stanford.slac.ssrl.authentication.utility.AuthGatewayBean) _jspx_page_context.getAttribute("gate", PageContext.SESSION_SCOPE);
        if (gate == null){
          gate = new edu.stanford.slac.ssrl.authentication.utility.AuthGatewayBean();
          _jspx_page_context.setAttribute("gate", gate, PageContext.SESSION_SCOPE);
        }
      }
      out.write('\r');
      out.write('\n');
      out.write("\r\n");
      out.write("\r\n");


//============================================================
// global variables (initialization)
g_session= session;
g_gate= gate;

if (gate == null)
	response.sendRedirect("loginForm.jsp");

ctsdb = SilUtil.getCassetteDB(); // singleton

      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write('\r');
      out.write('\n');
      out.write("\r\n");
      out.write("\r\n");

//============================================================
//============================================================
// server side script

String accessID= "" + ServletUtil.getSessionId(request);
String userName= "" + ServletUtil.getUserName(request);
String forCassetteID= ""+ request.getParameter("forCassetteID");

checkAccessID(request, response);
if((userName.length() == 0) || userName.equals("null") )
{
	userName = gate.getUserID();
}

String Login_Name= userName;

// server side script
//============================================================
//============================================================

      out.write("\r\n");
      out.write("\r\n");
      out.write("<HTML>\r\n");
      out.write("<head>\r\n");
      out.write("<title>Sample Database</title>\r\n");
      out.write("</head>\r\n");
      out.write("<BODY>\r\n");
      out.write("\r\n");

// pageheader.jsp
// define the header line for web pages of the Crystal Cassette Tracking System
// load different header depending on the installation site.

   String pageHeader = getConfigValue("pageheader");
   
   if ((pageHeader == null) || (pageHeader.length() == 0))
   	pageHeader = "ssrlheader.jsp";

      out.write("\r\n");
      out.write("\r\n");
      org.apache.jasper.runtime.JspRuntimeLibrary.include(request, response,  pageHeader , out, false);
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("<H2>Upload Excel File</H2>\r\n");
      out.write("\r\n");
      out.write("<FORM action=\"uploadSil.do\"\r\n");
      out.write("      enctype=\"multipart/form-data\"\r\n");
      out.write("      method=\"post\">\r\n");
      out.write("<INPUT name=\"accessID\" type=\"hidden\" value=\"");
      out.print(accessID );
      out.write("\" />\r\n");
      out.write("<INPUT name=\"userName\" type=\"hidden\" value=\"");
      out.print(userName );
      out.write("\" />\r\n");
      out.write("<INPUT name=\"forCassetteID\" type=\"hidden\" value=\"");
      out.print(forCassetteID );
      out.write("\" />\r\n");
      out.write("<INPUT name=\"format\" type=\"hidden\" value=\"html\" />\r\n");
      out.write("<P>\r\n");
      out.write("User Name: ");
      out.print(userName );
      out.write("\r\n");
      out.write("<BR>\r\n");
      out.write("<BR>\r\n");
      out.write("Excel File: <BR>\r\n");
      out.write("<INPUT type=\"file\" name=\"fileName\" size=\"60\"/><BR><BR>\r\n");
      out.write("Cassette PIN:\r\n");
      out.write("<INPUT type=\"text\" name=\"PIN_Number\" value=\"unknown\" size=\"18\"/><BR>\r\n");
      out.write("<p>\r\n");
      out.write("Please enter cassette PIN (inscribed on the cassette) if your spreadsheet does not\r\n");
      out.write("contain \"ContainerID\" column.<br/>\r\n");
      out.write("Examples: SSRL120 or Amgen.\r\n");
      out.write("</p>\r\n");
      out.write("Spreadsheet Name:\r\n");
      out.write("\r\n");
      out.write("<INPUT type=\"text\" name=\"forSheetName\" size=\"18\"\r\n");
 if( userName.equals("jcsg") )
{
 //value="beam_rpt"

      out.write("\r\n");
      out.write(" value=\"beam_rpt\"\r\n");

}
else
{
 //value="Sheet1"

      out.write("\r\n");
      out.write(" value=\"Sheet1\"\r\n");

}

      out.write("\r\n");
      out.write("/>\r\n");
      out.write("<BR><BR>\r\n");
 if (ServletUtil.isUserStaff(gate)) { 
      out.write("\r\n");
      out.write("Spreadsheet Type:\r\n");
      out.write("<select name=\"xslt\">\r\n");
      out.write("<option value=\"");
      out.print( userName );
      out.write("\">default</option>\r\n");
      out.write("<option value=\"jcsg\">jcsg</option>\r\n");
      out.write("</select>\r\n");
      out.write("<BR>\r\n");
 } 
      out.write("\r\n");
      out.write("\r\n");
      out.write("<BR>\r\n");
      out.write("Please note that generally the spreadsheet name is not the same as the file name.\r\n");
      out.write("In most cases is the spreadsheet name \"Sheet1\" but Microsoft Excel gives you the option to change it.\r\n");
      out.write("Please use only alphanumeric characters for the spreadsheet name and do not use any space characters.\r\n");
      out.write("<BR>\r\n");
      out.write("<BR>\r\n");
      out.write("<INPUT type=\"submit\" value=\"Upload\"/> <INPUT type=\"reset\"/>\r\n");
      out.write("</P>\r\n");
      out.write("</FORM>\r\n");
      out.write("\r\n");
      out.write("<BR>\r\n");
      out.write("\r\n");
      out.write("For more information see the\r\n");
      out.write("<A class=\"clsLinkX\" HREF=\"help.jsp\">\r\n");
      out.write("Online Help</A>.\r\n");
      out.write("<BR>\r\n");
      out.write("<BR>\r\n");
      out.write("\r\n");
      out.write("<HR>\r\n");
      out.write("<BR>\r\n");
      out.write("\r\n");
      out.write("<A HREF=\"CassetteInfo.jsp\">\r\n");
      out.write("Back</A> to the Sample Database.\r\n");
      out.write("<BR>\r\n");
      out.write("<BR>\r\n");
      out.write("\r\n");
      out.write("\r\n");
      out.write("</BODY>\r\n");
      out.write("</HTML>\r\n");
    } catch (Throwable t) {
      if (!(t instanceof SkipPageException)){
        out = _jspx_out;
        if (out != null && out.getBufferSize() != 0)
          out.clearBuffer();
        if (_jspx_page_context != null) _jspx_page_context.handlePageException(t);
      }
    } finally {
      if (_jspxFactory != null) _jspxFactory.releasePageContext(_jspx_page_context);
    }
  }
}
