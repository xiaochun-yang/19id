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

import webice.beans.*;
import webice.beans.autoindex.*;
import java.util.Vector;


public class IntegrateAdditionalSolutionsAction extends Action
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


		AutoindexViewer top = client.getAutoindexViewer();

		if (top == null)
			throw new ServletException("AutoindexViewer is null");

		AutoindexRun run = top.getSelectedRun();

		if (run == null)
			throw new ServletException("Selected run is null");

		RunController controller = run.getRunController();
		
		String sol = "";
		Vector sols = new Vector();
		for (int i = 0; i < 30; ++i) {
			if (request.getParameter(String.valueOf(i)) == null)
				continue;

			sols.add(new Integer(i));

		}
		

		controller.integrateAdditionalSolutions(sols);


		return mapping.findForward("success");


	}



}

