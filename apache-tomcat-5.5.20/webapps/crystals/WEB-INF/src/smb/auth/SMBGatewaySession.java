/**********************************************************************************
                        Copyright 2003
                              by
                 The Board of Trustees of the
               Leland Stanford Junior University
                      All rights reserved.


                       Disclaimer Notice

     The items furnished herewith were developed under the sponsorship
 of the U.S. Government.  Neither the U.S., nor the U.S. D.O.E., nor the
 Leland Stanford Junior University, nor their employees, makes any war-
 ranty, express or implied, or assumes any liability or responsibility
 for accuracy, completeness or usefulness of any information, apparatus,
 product or process disclosed, or represents that its use will not in-
 fringe privately-owned rights.  Mention of any product, its manufactur-
 er, or suppliers shall not, nor is it intended to, imply approval, dis-
 approval, or fitness for any particular use.  The U.S. and the Univer-
 sity at all times retain the right to use and disseminate the furnished
 items for any purpose whatsoever.                       Notice 91 02 01

   Work supported by the U.S. Department of Energy under contract
   DE-AC03-76SF00515; and the National Institutes of Health, National
   Center for Research Resources, grant 2P41RR01209.


                       Permission Notice

 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTA-
 BILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*********************************************************************************/
package smb.auth;

import java.io.*;
import java.net.*;
import edu.stanford.slac.ssrl.authentication.thirdparty.Base64;

/**
 * SMBGatewaySession is a utility class for the SMB implementation of the
 * authentication server. It enables the developer to create, check, and
 * end an SMB authentication session without making direct HTTP calls.
 *
 * @author Kenneth Sharp
 * @version 2.0 (released September 15, 2003)
 */
public class SMBGatewaySession {

    private String SMBSessionID = "";
    private boolean sessionValid = false;
    private String sessionCreation = "";
    private String sessionLastAccessed = "";
    private String userID = "";
    private String userName = "";
    private String userType = "";
    private String beamlines = "";
    private int userPriv = 0;
    private boolean updateSessionOK = false;
    private String updateError = "";
    private boolean bl[];
    private String appName = null;
    private boolean dbAuth = false;
    private boolean userStaff = false;
    private String phoneNumber = "";
    private String jobTitle = "";
    private boolean remoteAccess = false;
    private boolean enabled = false;
    private String allBeamlines = "";

    // host for normal use
    private String servletHost = "http://smb.slac.stanford.edu:8180/gateway/servlet/";

    public SMBGatewaySession()
    {
    }

   
    /**
     * Constructor to create an object based on an existing session.
     *
     * @param String SessionID is the existing session's id
     * @param String appName is the application instantiating this bean.
     */
    public SMBGatewaySession(String SMBSessionID, String appName) 
    {
        this(null, SMBSessionID, appName);

    }
 

    public SMBGatewaySession(String url, String SMBSessionID, String appName)
    {
		if ((url != null) && (url.length() > 0))
			this.servletHost = url;
       		this.SMBSessionID = SMBSessionID;
        	this.appName = appName;
        	if (this.SMBSessionID == null) 
			this.SMBSessionID = "";
			
        	bl = new boolean[7];
        	for (int i=0; i<7; i++) bl[i] = false;
        	updateSessionOK = updateSessionData(true);
     }

     /**
     * Constructor to create an object based on an existing session.
     *
     * @param String SessionID is the existing session's id
     * @param String appName is the application instantiating this bean.
     */
    public SMBGatewaySession(String appName, String userid, String password,
                             boolean dbAuth)
    {
		this(null, appName, userid, password, dbAuth);
    }


    /**
     * Constructor to create a new session.
     *
     * @param String appName is the application instantiating this bean.
     * @param String userid is the user to be authenticated (in clear text)
     * @param String password is the user's password (in clear text)
     * @param boolean dbAuth is true if the application wishes to use the User
     *		Resource Management database; false if the application wishes to use
     *		the beamline configuration database.
     */
    public SMBGatewaySession(String url, String appName, String userid, String password,
                             boolean dbAuth) 
    {

		if ((url != null) && (url.length() > 0))
			this.servletHost = url;
       this.userID = userid;
        this.appName = appName;
        this.dbAuth = dbAuth;

        // get the password, and encode it using Base64 encoding
        String passwd = Base64.encode(userid + ":" + password);
        //String passwd = encode(userid + ":" + password);
        // change any "=" to "%3D" for placement into a URL
        while (passwd.lastIndexOf((char)0x3D) > -1) {
            int idx = passwd.lastIndexOf((char)0x3D);
            passwd = passwd.substring(0, idx) + "%3D" + passwd.substring(idx+1);
        }

        // Build URL
        // create the APPLOGIN url with the user name and password,
        // and the authentication method (dbAuth=True to use the test user db)
        String myURL = servletHost + "APPLOGIN?userid=" + userid + "&passwd=" + passwd + "&AppName=" + appName;
        if (dbAuth) myURL = myURL.concat("&AuthMethod=smb_urm2");

        // try logging in the user and reading the response headers
        try {
            URL newUrl = new URL(myURL);
            HttpURLConnection urlConn = (HttpURLConnection) newUrl.openConnection();
            int response = urlConn.getResponseCode();
            if (response == 200) {
                // look for the SMBSessionID in the response cookies
                String cookieString = null;
                int j=0;
                int i=0;
                do {
                    cookieString = urlConn.getHeaderField(j++);
                    i = cookieString.indexOf("SMBSessionID=");
                    if (i > -1) {
                        this.SMBSessionID = cookieString.substring(13,45);
                        cookieString = null;
                    }
                } while (cookieString != null);
                if (this.SMBSessionID == null) this.SMBSessionID = "";
                if (this.SMBSessionID.length() > 0) {
                    this.sessionValid = true;
                    this.sessionCreation = urlConn.getHeaderField("Auth.SessionCreation");
                    this.sessionLastAccessed = urlConn.getHeaderField("Auth.SessionAccessed");
                    this.userName = urlConn.getHeaderField("Auth.UserName");
                    this.userType = urlConn.getHeaderField("Auth.UserType");
                    this.beamlines = urlConn.getHeaderField("Auth.Beamlines");
                    this.userPriv = urlConn.getHeaderFieldInt("Auth.UserPriv", 0);
                    this.phoneNumber = urlConn.getHeaderField("Auth.OfficePhone");
                    this.jobTitle = urlConn.getHeaderField("Auth.JobTitle");
                    this.remoteAccess = urlConn.getHeaderField("Auth.RemoteAccess").equalsIgnoreCase("Y");
                    this.enabled = urlConn.getHeaderField("Auth.Enabled").equalsIgnoreCase("Y");
                    String staffStr = urlConn.getHeaderField("Auth.UserStaff");
                    this.userStaff = staffStr.equalsIgnoreCase("Y");
                    this.updateSessionOK = true;
                    this.updateError = "";
                    bl = new boolean[7];
                    if (beamlines.indexOf("bl1-5") > -1) bl[0] = true;
                    if (beamlines.indexOf("bl7-1") > -1) bl[1] = true;
                    if (beamlines.indexOf("bl9-1") > -1) bl[2] = true;
                    if (beamlines.indexOf("bl9-2") > -1) bl[3] = true;
                    if (beamlines.indexOf("bl11-1") > -1) bl[4] = true;
                    if (beamlines.indexOf("bl11-3") > -1) bl[5] = true;
                    if (beamlines.indexOf("ALL") > -1) {
                     	for (int k=0; k<6; k++) bl[k] = true;
                    }
                    this.allBeamlines = urlConn.getHeaderField("Auth.AllBeamlines");
                } else {
                    this.updateError = "Unable to find SMBSessionID Cookie in response.";
                }
            } else {
                this.updateError = "Invalid Response Code from APPLOGIN: " + response;
            }
        } catch (MalformedURLException e) {
        } catch (IOException e) {}
    }

    /**
     * Ends the current session by calling the EndSession applet.
     */
    public void endSession() {

        if (SMBSessionID.length() < 1) {
            updateError = "SMBSessionID is blank.";
            return;
        }

        // Build URL
        String myURL = servletHost + "EndSession;jsessionid=";
        myURL = myURL.concat(SMBSessionID);
        myURL = myURL.concat("?AppName=" + appName);
        // query the URL
        try {
            URL newUrl = new URL(myURL);
            HttpURLConnection urlConn = (HttpURLConnection) newUrl.openConnection();
            int response = urlConn.getResponseCode();
        } catch (MalformedURLException e) { updateError = "MalformedURLException";
        } catch (IOException e) {updateError = "IOException"; }
    }

    /**
     * Updates the session data by calling the SessionStatus servlet.
     * Also rechecks the database for beamline access if the recheckDatabase
     * parameter is true.
     *
     * @param boolean recheckDatabase forces a recheck of beamline access if true.
     * @return <code>true</code> if the update is performed successfully.
     * returns <code>false</otherwise>.
     */
    public boolean updateSessionData(boolean recheckDatabase) {

        updateSessionOK = false;
        if (SMBSessionID.length() < 1) {
            updateError = "SMBSessionID is blank.";
            return false;
        }

        // Build URL
        String myURL = servletHost + "SessionStatus;jsessionid=";
        myURL = myURL.concat(SMBSessionID);
        myURL = myURL.concat("?AppName=" + appName);
        if (recheckDatabase) {
            myURL = myURL.concat("&ValidBeamlines=True");
        }

        // query the URL
        try {
            URL newUrl = new URL(myURL);
            HttpURLConnection urlConn = (HttpURLConnection) newUrl.openConnection();
            int response = urlConn.getResponseCode();
            if (response != 200) {
                updateError = "Unable to query SessionStatus servlet. Code " + response;
                return false;
            }
            String headerVal;
            headerVal = urlConn.getHeaderField("Auth.SessionValid");
            sessionValid = false;
            if (headerVal != null) sessionValid = headerVal.equals("TRUE");
            if (!sessionValid) {
            	updateError = "The requested session is invalid.";
            	return false;
            }
            headerVal = urlConn.getHeaderField("Auth.SessionCreation");
            sessionCreation = "";
            if (headerVal != null) sessionCreation = headerVal;
            headerVal = urlConn.getHeaderField("Auth.SessionAccessed");
            sessionLastAccessed = "";
            if (headerVal != null) sessionLastAccessed = headerVal;
            headerVal = urlConn.getHeaderField("Auth.UserID");
            userID = "";
            if (headerVal != null) userID = headerVal;
            headerVal = urlConn.getHeaderField("Auth.UserName");
            userName = "";
            if (headerVal != null) userName = headerVal;
            headerVal = urlConn.getHeaderField("Auth.UserType");
            userType = "";
            if (headerVal != null) userType = headerVal;
            headerVal = urlConn.getHeaderField("Auth.Beamlines");
            beamlines = "";
            if (headerVal != null) beamlines = headerVal;
            userPriv = urlConn.getHeaderFieldInt("Auth.UserPriv", 0);
            if (beamlines.indexOf("bl1-5") > -1) bl[0] = true;
            if (beamlines.indexOf("bl7-1") > -1) bl[1] = true;
            if (beamlines.indexOf("bl9-1") > -1) bl[2] = true;
            if (beamlines.indexOf("bl9-2") > -1) bl[3] = true;
            if (beamlines.indexOf("bl11-1") > -1) bl[4] = true;
            if (beamlines.indexOf("bl11-3") > -1) bl[5] = true;
            if (beamlines.indexOf("ALL") > -1) {
 	          	for (int k=0; k<6; k++) bl[k] = true;
            }
            headerVal = urlConn.getHeaderField("Auth.DBAuth");
            if (headerVal != null && headerVal.equalsIgnoreCase("True")) dbAuth = true;
            headerVal = urlConn.getHeaderField("Auth.UserStaff");
            userStaff = false;
            if (headerVal != null) userStaff = headerVal.equalsIgnoreCase("Y");
            headerVal = urlConn.getHeaderField("Auth.Enabled");
            enabled = false;
            if (headerVal != null) enabled = headerVal.equalsIgnoreCase("Y");
            headerVal = urlConn.getHeaderField("Auth.RemoteAccess");
            remoteAccess = false;
            if (headerVal != null) remoteAccess = headerVal.equalsIgnoreCase("Y");
            headerVal = urlConn.getHeaderField("Auth.OfficePhone");
            phoneNumber = "";
            if (headerVal != null) phoneNumber = headerVal;
            headerVal = urlConn.getHeaderField("Auth.JobTitle");
            jobTitle = "";
            if (headerVal != null) jobTitle = headerVal;
            headerVal = urlConn.getHeaderField("Auth.AllBeamlines");
            allBeamlines = "";
            if (headerVal != null) allBeamlines = headerVal;
            updateSessionOK = true;
            updateError = "";
        } catch (MalformedURLException e) {
        	updateError = "MalformedURLException";
        } catch (IOException e) {
        	updateError = "IOException";
        }
        return updateSessionOK;
        //} finally {
        //    return updateSessionOK;
        //}
    }

    /**
     * @return String containing the SMBSessionID.
     */
    public String getSessionID() { return SMBSessionID;}

    /**
     * @return <code>true</code> if session is valid, <code>false</code> otherwise.
     */
    public boolean isSessionValid() { return sessionValid;}
    /**
     * @return String containing time (in milliseconds) that session was created.
     */
    public String getCreationTime() {return sessionCreation;}
    /**
     * @return String containing time (in milliseconds) that session was last accessed.
     */
    public String getLastAccessTime() {return sessionLastAccessed;}
    /**
     * @return String containing userid used to login to session.
     */
    public String getUserID() { return userID;}
    /**
     * @return String containing display name of the user.
     */
    public String getUserName() {return userName;}
    /**
     * @return String containing user type (UNIX, WEB, and/or NT) of user.
     */
    public String getUserType() {return userType;}
    /**
     * @return String containing beamlines accessible to the user.
     */
    public String getBeamlines() {return beamlines;}
    /**
     * @return int containing privilege level of the user. Deprecated. Always returns 0.
     */
    public int getUserPriv() {return userPriv;}
    /**
     * @return <code>true</code> if last update was successful, <code>false</code> otherwise.
     */
    public boolean isUpdateSuccessful() {return updateSessionOK;}
    /**
     * @return String containing most recent error.
     */
    public String getUpdateError() {return updateError;}
    /**
     * @param i indicating which beamline to check where 1=1-5, 2=7-1, 3=9-1, 4=9-2, 5=11-1
     * @return <code>true</code> if beamline is accessible, <code>false</code> otherwise
     */
    public boolean getBL(int i) {return bl[i];}
    /**
     * @return <code>true</code> if session is authenticated against the User Resource
     * database, and <code>false</code> if session is authenticated against Beamline
     * Configuration database.
     */
    public boolean getAuth() {return dbAuth;}
    /**
     * @return <code>true</code> if user is staff, <code>false</code> otherwise.
     */
    public boolean getUserStaff() { return userStaff;}

    /**
     * @return <code>true</code> if user is enabled, <code>false</code> otherwise.
     */
    public boolean getUserEnabled() { return enabled;}

    /**
     * @return <code>true</code> if user has remote access, <code>false</code> otherwise.
     */
    public boolean getUserRemoteAccess() { return remoteAccess; }

    /**
     * @return String containing the user's office phone number.
     */
    public String getUserOfficePhone() { return phoneNumber; }

    /**
     * @return String containing the user's job title.
     */
    public String getUserJobTitle() { return jobTitle; }

    /**
     * @return String containing the all available beamlines.
     */
    public String getAllBeamlines() { return allBeamlines; }
 /*
    private String encode(String data)
    {
        return(getString(encode(getBinaryBytes(data))));
    }

    private byte[] encode(byte[] data)
    {
        int c;
        int len = data.length;
        StringBuffer ret = new StringBuffer(((len / 3) + 1) * 4);
        for (int i = 0; i < len; ++i)
        {
            c = (data[i] >> 2) & 0x3f;
            ret.append(cvt.charAt(c));
            c = (data[i] << 4) & 0x3f;
            if (++i < len)
                c |= (data[i] >> 4) & 0x0f;

            ret.append(cvt.charAt(c));
            if (i < len)
            {
                c = (data[i] << 2) & 0x3f;
                if (++i < len)
                    c |= (data[i] >> 6) & 0x03;

                ret.append(cvt.charAt(c));
            }
            else
            {
                ++i;
                ret.append((char) fillchar);
            }

            if (i < len)
            {
                c = data[i] & 0x3f;
                ret.append(cvt.charAt(c));
            }
            else
            {
                ret.append((char) fillchar);
            }
        }

        return(getBinaryBytes(ret.toString()));
    }


    private String getString(byte[] arr)
    {
        StringBuffer buf = new StringBuffer();
        for (int i = 0; i < arr.length; ++i)
            buf.append((char) arr[i]);

        return(buf.toString());
    }

    private byte[] getBinaryBytes(String str)
    {
        byte[] b = new byte[str.length()];
        for (int i = 0; i < b.length; ++i)
            b[i] = (byte) str.charAt(i);

        return(b);
    }

    private final int    fillchar = '=';

                                    // 00000000001111111111222222
                                    // 01234567890123456789012345
    private final String cvt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

                                    // 22223333333333444444444455
                                    // 67890123456789012345678901
                                    + "abcdefghijklmnopqrstuvwxyz"

                                    // 555555556666
                                    // 234567890123
                                    + "0123456789+/"; */


}
