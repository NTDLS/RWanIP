<%Option Explicit%>
<%
	Response.CacheControl = "no-cache"
	Response.AddHeader "Pragma", "no-cache"
	Response.Expires = -1
%>
<HTML>
    <Head>
        <META HTTP-EQUIV="REFRESH" CONTENT="10">
        <Title>IPID</Title>
    </Head>
    <Body BGColor="#FFFFFF">
    <Center>
		<Table Border="1" CellPadding="0" CellSpacing="0" Width="400">
		<TR><TD>
			<Table Border="0" Width="100%" CellPadding="1" CellSpacing="0">
			<TR>
				<TD BGColor="#cccccc" ColSpan="2">
					<Center><B>WAN IP Address<B></Center>
				</TD>
			</TR>
			<TR bgcolor="#eeeeee">
				<TD Align="Right">
					<B>IP Address:</B>
				</TD>
				<TD Align="Left">
					<font color="#aa0000"><IP><%=Request.ServerVariables("REMOTE_ADDR")%></IP></font>
				</TD>
			</TR>
			<TR bgcolor="#eeeeee">
				<TD Align="Right">
					<B>As of:</B>
				</TD>
				<TD Align="Left">
					<AsOf><%Response.Write Now()%></AsOf>
				</TD>
			</TR>
			</Table>
		</TD></TR>
		</Table>
    </Center>
    </Body>
</HTML>
