/**
 * Javabean for SMB resources
 */
package webice.actions.autoindex;

import java.io.IOException;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;
import java.util.Vector;

import webice.beans.*;
import webice.beans.autoindex.*;


public class SelectImagesAction extends Action
{

	public ActionForward execute(ActionMapping mapping,
							ActionForm f,
							HttpServletRequest request,
							HttpServletResponse response)
				throws Exception
	{


		HttpSession session = request.getSession();

		Client client = (Client)session.getAttribute("client");

		if (client == null)
			throw new NullClientException("Client is null");


		AutoindexViewer viewer = client.getAutoindexViewer();

		if (viewer == null)
			throw new ServletException("AutoindexViewer is null");

		AutoindexRun run = viewer.getSelectedRun();

		if (run == null)
			throw new ServletException("Selected node is null");

		RunController controller = run.getRunController();

		controller.resetLog();

		if (controller.getStatus() > RunController.SETUP) {
			controller.appendLog("ERROR: Setup cannot be modified");
			return mapping.findForward("success");
		}

		Object files[] = viewer.getFileBrowser().getFileNames();

		controller.clearImages();

		controller.resetLog();

		String file = null;
		Vector selectedFiles = new Vector();
		for (int i = 0; i < files.length; ++i) {
			file = (String)files[i];
			if (request.getParameter(file) != null)
				selectedFiles.add(file);
		}
		
		try {

		if (selectedFiles.size() == 2) {
			controller.setImages((String)selectedFiles.elementAt(0),
						(String)selectedFiles.elementAt(1));
			// Done with the file selection
			run.setShowFileBrowser(false);
		} else {
			request.setAttribute("error", "Please select 2 images");
			controller.setLog("Error: Please select 2 images\n");
		}
		
		} catch (Exception e) {
			request.setAttribute("error", e.getMessage());
			controller.setLog(e.getMessage());
			WebiceLogger.warn("Failed in SelectImagesAction: " + e.getMessage());
		}


		return mapping.findForward("success");


	}



}

