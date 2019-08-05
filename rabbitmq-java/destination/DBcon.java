package destination;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class DBcon {

	private String driver = "org.mariadb.jdbc.Driver";
	private Connection con;

	public DBcon() {
		try {
			Class.forName(driver);
			String url = "jdbc:mariadb://127.0.0.1:3306/orademo";
			con = DriverManager.getConnection(url, "seongbeom", "1234");

			if (con != null) {
				System.out.println("DB 접속 성공");
			}

		} catch (ClassNotFoundException e) {
			System.out.println("드라이버 로드 실패");
		} catch (SQLException e) {
			System.out.println("DB 접속 실패");
			e.printStackTrace();
		}
	}

	public void createStatement(String message1) {
		try {
			//Statement st = con.createStatement();
			PreparedStatement pst = null;
			StringBuilder sb = new StringBuilder();
			String data = sb.append("INSERT INTO rabbit(message) " + "VALUE(" + message1 + ")").toString();
			//st.executeUpdate(data);
			pst = con.prepareStatement(data);
			//pst.setString(7, message1);
			ResultSet r = pst.executeQuery();
			System.out.println("변경된 row : " + r);

		} catch (Exception e) {
			System.err.println("Got an exception!");
			System.err.println(e.getMessage());
			e.printStackTrace();
		}

	}

	public void disconnect() throws SQLException {
		con.close();
	}
}
