import asyncore
import email.utils
import socket
import smtpd
import smtplib
import io
import sys
import time
import select

import unittest
from test import support, mock_socket

try:
    import threading
except ImportError:
    threading = None

HOST = support.HOST

if sys.platform == 'darwin':
    # select.poll returns a select.POLLHUP at the end of the tests
    # on darwin, so just ignore it
    def handle_expt(self):
        pass
    smtpd.SMTPChannel.handle_expt = handle_expt


def server(evt, buf, serv):
    serv.listen(5)
    evt.set()
    try:
        conn, addr = serv.accept()
    except socket.timeout:
        pass
    else:
        n = 500
        while buf and n > 0:
            r, w, e = select.select([], [conn], [])
            if w:
                sent = conn.send(buf)
                buf = buf[sent:]

            n -= 1

        conn.close()
    finally:
        serv.close()
        evt.set()

class GeneralTests(unittest.TestCase):

    def setUp(self):
        smtplib.socket = mock_socket
        self.port = 25

    def tearDown(self):
        smtplib.socket = socket

    def testBasic1(self):
        mock_socket.reply_with(b"220 Hola mundo")
        # connects
        smtp = smtplib.SMTP(HOST, self.port)
        smtp.close()

    def testBasic2(self):
        mock_socket.reply_with(b"220 Hola mundo")
        # connects, include port in host name
        smtp = smtplib.SMTP("%s:%s" % (HOST, self.port))
        smtp.close()

    def testLocalHostName(self):
        mock_socket.reply_with(b"220 Hola mundo")
        # check that supplied local_hostname is used
        smtp = smtplib.SMTP(HOST, self.port, local_hostname="testhost")
        self.assertEqual(smtp.local_hostname, "testhost")
        smtp.close()

    def testTimeoutDefault(self):
        mock_socket.reply_with(b"220 Hola mundo")
        self.assertTrue(mock_socket.getdefaulttimeout() is None)
        mock_socket.setdefaulttimeout(30)
        self.assertEqual(mock_socket.getdefaulttimeout(), 30)
        try:
            smtp = smtplib.SMTP(HOST, self.port)
        finally:
            mock_socket.setdefaulttimeout(None)
        self.assertEqual(smtp.sock.gettimeout(), 30)
        smtp.close()

    def testTimeoutNone(self):
        mock_socket.reply_with(b"220 Hola mundo")
        self.assertTrue(socket.getdefaulttimeout() is None)
        socket.setdefaulttimeout(30)
        try:
            smtp = smtplib.SMTP(HOST, self.port, timeout=None)
        finally:
            socket.setdefaulttimeout(None)
        self.assertTrue(smtp.sock.gettimeout() is None)
        smtp.close()

    def testTimeoutValue(self):
        mock_socket.reply_with(b"220 Hola mundo")
        smtp = smtplib.SMTP(HOST, self.port, timeout=30)
        self.assertEqual(smtp.sock.gettimeout(), 30)
        smtp.close()


# Test server thread using the specified SMTP server class
def debugging_server(serv, serv_evt, client_evt):
    serv_evt.set()

    try:
        if hasattr(select, 'poll'):
            poll_fun = asyncore.poll2
        else:
            poll_fun = asyncore.poll

        n = 1000
        while asyncore.socket_map and n > 0:
            poll_fun(0.01, asyncore.socket_map)

            # when the client conversation is finished, it will
            # set client_evt, and it's then ok to kill the server
            if client_evt.is_set():
                serv.close()
                break

            n -= 1

    except socket.timeout:
        pass
    finally:
        if not client_evt.is_set():
            # allow some time for the client to read the result
            time.sleep(0.5)
            serv.close()
        asyncore.close_all()
        serv_evt.set()

MSG_BEGIN = '---------- MESSAGE FOLLOWS ----------\n'
MSG_END = '------------ END MESSAGE ------------\n'

# NOTE: Some SMTP objects in the tests below are created with a non-default
# local_hostname argument to the constructor, since (on some systems) the FQDN
# lookup caused by the default local_hostname sometimes takes so long that the
# test server times out, causing the test to fail.

# Test behavior of smtpd.DebuggingServer
@unittest.skipUnless(threading, 'Threading required for this test.')
class DebuggingServerTests(unittest.TestCase):

    def setUp(self):
        self.real_getfqdn = socket.getfqdn
        socket.getfqdn = mock_socket.getfqdn
        # temporarily replace sys.stdout to capture DebuggingServer output
        self.old_stdout = sys.stdout
        self.output = io.StringIO()
        sys.stdout = self.output

        self._threads = support.threading_setup()
        self.serv_evt = threading.Event()
        self.client_evt = threading.Event()
        # Pick a random unused port by passing 0 for the port number
        self.serv = smtpd.DebuggingServer((HOST, 0), ('nowhere', -1))
        # Keep a note of what port was assigned
        self.port = self.serv.socket.getsockname()[1]
        serv_args = (self.serv, self.serv_evt, self.client_evt)
        self.thread = threading.Thread(target=debugging_server, args=serv_args)
        self.thread.start()

        # wait until server thread has assigned a port number
        self.serv_evt.wait()
        self.serv_evt.clear()

    def tearDown(self):
        socket.getfqdn = self.real_getfqdn
        # indicate that the client is finished
        self.client_evt.set()
        # wait for the server thread to terminate
        self.serv_evt.wait()
        self.thread.join()
        support.threading_cleanup(*self._threads)
        # restore sys.stdout
        sys.stdout = self.old_stdout

    def testBasic(self):
        # connect
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=3)
        smtp.quit()

    def testNOOP(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=3)
        expected = (250, b'Ok')
        self.assertEqual(smtp.noop(), expected)
        smtp.quit()

    def testRSET(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=3)
        expected = (250, b'Ok')
        self.assertEqual(smtp.rset(), expected)
        smtp.quit()

    def testNotImplemented(self):
        # EHLO isn't implemented in DebuggingServer
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=3)
        expected = (502, b'Error: command "EHLO" not implemented')
        self.assertEqual(smtp.ehlo(), expected)
        smtp.quit()

    def testVRFY(self):
        # VRFY isn't implemented in DebuggingServer
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=3)
        expected = (502, b'Error: command "VRFY" not implemented')
        self.assertEqual(smtp.vrfy('nobody@nowhere.com'), expected)
        self.assertEqual(smtp.verify('nobody@nowhere.com'), expected)
        smtp.quit()

    def testSecondHELO(self):
        # check that a second HELO returns a message that it's a duplicate
        # (this behavior is specific to smtpd.SMTPChannel)
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=3)
        smtp.helo()
        expected = (503, b'Duplicate HELO/EHLO')
        self.assertEqual(smtp.helo(), expected)
        smtp.quit()

    def testHELP(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=3)
        self.assertEqual(smtp.help(), b'Error: command "HELP" not implemented')
        smtp.quit()

    def testSend(self):
        # connect and send mail
        m = 'A test message'
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=3)
        smtp.sendmail('John', 'Sally', m)
        # XXX(nnorwitz): this test is flaky and dies with a bad file descriptor
        # in asyncore.  This sleep might help, but should really be fixed
        # properly by using an Event variable.
        time.sleep(0.01)
        smtp.quit()

        self.client_evt.set()
        self.serv_evt.wait()
        self.output.flush()
        mexpect = '%s%s\n%s' % (MSG_BEGIN, m, MSG_END)
        self.assertEqual(self.output.getvalue(), mexpect)


class NonConnectingTests(unittest.TestCase):

    def setUp(self):
        smtplib.socket = mock_socket

    def tearDown(self):
        smtplib.socket = socket

    def testNotConnected(self):
        # Test various operations on an unconnected SMTP object that
        # should raise exceptions (at present the attempt in SMTP.send
        # to reference the nonexistent 'sock' attribute of the SMTP object
        # causes an AttributeError)
        smtp = smtplib.SMTP()
        self.assertRaises(smtplib.SMTPServerDisconnected, smtp.ehlo)
        self.assertRaises(smtplib.SMTPServerDisconnected,
                          smtp.send, 'test msg')

    def testNonnumericPort(self):
        # check that non-numeric port raises socket.error
        self.assertRaises(mock_socket.error, smtplib.SMTP,
                          "localhost", "bogus")
        self.assertRaises(mock_socket.error, smtplib.SMTP,
                          "localhost:bogus")


# test response of client to a non-successful HELO message
@unittest.skipUnless(threading, 'Threading required for this test.')
class BadHELOServerTests(unittest.TestCase):

    def setUp(self):
        smtplib.socket = mock_socket
        mock_socket.reply_with(b"199 no hello for you!")
        self.old_stdout = sys.stdout
        self.output = io.StringIO()
        sys.stdout = self.output
        self.port = 25

    def tearDown(self):
        smtplib.socket = socket
        sys.stdout = self.old_stdout

    def testFailingHELO(self):
        self.assertRaises(smtplib.SMTPConnectError, smtplib.SMTP,
                            HOST, self.port, 'localhost', 3)


sim_users = {'Mr.A@somewhere.com':'John A',
             'Ms.B@somewhere.com':'Sally B',
             'Mrs.C@somewhereesle.com':'Ruth C',
            }

sim_auth = ('Mr.A@somewhere.com', 'somepassword')
sim_cram_md5_challenge = ('PENCeUxFREJoU0NnbmhNWitOMjNGNn'
                          'dAZWx3b29kLmlubm9zb2Z0LmNvbT4=')
sim_auth_credentials = {
    'login': 'TXIuQUBzb21ld2hlcmUuY29t',
    'plain': 'AE1yLkFAc29tZXdoZXJlLmNvbQBzb21lcGFzc3dvcmQ=',
    'cram-md5': ('TXIUQUBZB21LD2HLCMUUY29TIDG4OWQ0MJ'
                 'KWZGQ4ODNMNDA4NTGXMDRLZWMYZJDMODG1'),
    }
sim_auth_login_password = 'C29TZXBHC3N3B3JK'

sim_lists = {'list-1':['Mr.A@somewhere.com','Mrs.C@somewhereesle.com'],
             'list-2':['Ms.B@somewhere.com',],
            }

# Simulated SMTP channel & server
class SimSMTPChannel(smtpd.SMTPChannel):

    def __init__(self, extra_features, *args, **kw):
        self._extrafeatures = ''.join(
            [ "250-{0}\r\n".format(x) for x in extra_features ])
        super(SimSMTPChannel, self).__init__(*args, **kw)

    def smtp_EHLO(self, arg):
        resp = ('250-testhost\r\n'
                '250-EXPN\r\n'
                '250-SIZE 20000000\r\n'
                '250-STARTTLS\r\n'
                '250-DELIVERBY\r\n')
        resp = resp + self._extrafeatures + '250 HELP'
        self.push(resp)

    def smtp_VRFY(self, arg):
        raw_addr = email.utils.parseaddr(arg)[1]
        quoted_addr = smtplib.quoteaddr(arg)
        if raw_addr in sim_users:
            self.push('250 %s %s' % (sim_users[raw_addr], quoted_addr))
        else:
            self.push('550 No such user: %s' % arg)

    def smtp_EXPN(self, arg):
        list_name = email.utils.parseaddr(arg)[1].lower()
        if list_name in sim_lists:
            user_list = sim_lists[list_name]
            for n, user_email in enumerate(user_list):
                quoted_addr = smtplib.quoteaddr(user_email)
                if n < len(user_list) - 1:
                    self.push('250-%s %s' % (sim_users[user_email], quoted_addr))
                else:
                    self.push('250 %s %s' % (sim_users[user_email], quoted_addr))
        else:
            self.push('550 No access for you!')

    def smtp_AUTH(self, arg):
        if arg.strip().lower()=='cram-md5':
            self.push('334 {}'.format(sim_cram_md5_challenge))
            return
        mech, auth = arg.split()
        mech = mech.lower()
        if mech not in sim_auth_credentials:
            self.push('504 auth type unimplemented')
            return
        if mech == 'plain' and auth==sim_auth_credentials['plain']:
            self.push('235 plain auth ok')
        elif mech=='login' and auth==sim_auth_credentials['login']:
            self.push('334 Password:')
        else:
            self.push('550 No access for you!')

    def handle_error(self):
        raise


class SimSMTPServer(smtpd.SMTPServer):

    def __init__(self, *args, **kw):
        self._extra_features = []
        smtpd.SMTPServer.__init__(self, *args, **kw)

    def handle_accepted(self, conn, addr):
        self._SMTPchannel = SimSMTPChannel(self._extra_features,
                                           self, conn, addr)

    def process_message(self, peer, mailfrom, rcpttos, data):
        pass

    def add_feature(self, feature):
        self._extra_features.append(feature)

    def handle_error(self):
        raise


# Test various SMTP & ESMTP commands/behaviors that require a simulated server
# (i.e., something with more features than DebuggingServer)
@unittest.skipUnless(threading, 'Threading required for this test.')
class SMTPSimTests(unittest.TestCase):

    def setUp(self):
        self.real_getfqdn = socket.getfqdn
        socket.getfqdn = mock_socket.getfqdn
        self._threads = support.threading_setup()
        self.serv_evt = threading.Event()
        self.client_evt = threading.Event()
        # Pick a random unused port by passing 0 for the port number
        self.serv = SimSMTPServer((HOST, 0), ('nowhere', -1))
        # Keep a note of what port was assigned
        self.port = self.serv.socket.getsockname()[1]
        serv_args = (self.serv, self.serv_evt, self.client_evt)
        self.thread = threading.Thread(target=debugging_server, args=serv_args)
        self.thread.start()

        # wait until server thread has assigned a port number
        self.serv_evt.wait()
        self.serv_evt.clear()

    def tearDown(self):
        socket.getfqdn = self.real_getfqdn
        # indicate that the client is finished
        self.client_evt.set()
        # wait for the server thread to terminate
        self.serv_evt.wait()
        self.thread.join()
        support.threading_cleanup(*self._threads)

    def testBasic(self):
        # smoke test
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)
        smtp.quit()

    def testEHLO(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        # no features should be present before the EHLO
        self.assertEqual(smtp.esmtp_features, {})

        # features expected from the test server
        expected_features = {'expn':'',
                             'size': '20000000',
                             'starttls': '',
                             'deliverby': '',
                             'help': '',
                             }

        smtp.ehlo()
        self.assertEqual(smtp.esmtp_features, expected_features)
        for k in expected_features:
            self.assertTrue(smtp.has_extn(k))
        self.assertFalse(smtp.has_extn('unsupported-feature'))
        smtp.quit()

    def testVRFY(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        for email, name in sim_users.items():
            expected_known = (250, bytes('%s %s' %
                                         (name, smtplib.quoteaddr(email)),
                                         "ascii"))
            self.assertEqual(smtp.vrfy(email), expected_known)

        u = 'nobody@nowhere.com'
        expected_unknown = (550, ('No such user: %s'
                                       % smtplib.quoteaddr(u)).encode('ascii'))
        self.assertEqual(smtp.vrfy(u), expected_unknown)
        smtp.quit()

    def testEXPN(self):
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        for listname, members in sim_lists.items():
            users = []
            for m in members:
                users.append('%s %s' % (sim_users[m], smtplib.quoteaddr(m)))
            expected_known = (250, bytes('\n'.join(users), "ascii"))
            self.assertEqual(smtp.expn(listname), expected_known)

        u = 'PSU-Members-List'
        expected_unknown = (550, b'No access for you!')
        self.assertEqual(smtp.expn(u), expected_unknown)
        smtp.quit()

    def testAUTH_PLAIN(self):
        self.serv.add_feature("AUTH PLAIN")
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        expected_auth_ok = (235, b'plain auth ok')
        self.assertEqual(smtp.login(sim_auth[0], sim_auth[1]), expected_auth_ok)
        smtp.close()

    # SimSMTPChannel doesn't fully support LOGIN or CRAM-MD5 auth because they
    # require a synchronous read to obtain the credentials...so instead smtpd
    # sees the credential sent by smtplib's login method as an unknown command,
    # which results in smtplib raising an auth error.  Fortunately the error
    # message contains the encoded credential, so we can partially check that it
    # was generated correctly (partially, because the 'word' is uppercased in
    # the error message).

    def testAUTH_LOGIN(self):
        self.serv.add_feature("AUTH LOGIN")
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)
        try: smtp.login(sim_auth[0], sim_auth[1])
        except smtplib.SMTPAuthenticationError as err:
            self.assertIn(sim_auth_login_password, str(err))
        smtp.close()

    def testAUTH_CRAM_MD5(self):
        self.serv.add_feature("AUTH CRAM-MD5")
        smtp = smtplib.SMTP(HOST, self.port, local_hostname='localhost', timeout=15)

        try: smtp.login(sim_auth[0], sim_auth[1])
        except smtplib.SMTPAuthenticationError as err:
            self.assertIn(sim_auth_credentials['cram-md5'], str(err))
        smtp.close()

    #TODO: add tests for correct AUTH method fallback now that the
    #test infrastructure can support it.


def test_main(verbose=None):
    support.run_unittest(GeneralTests, DebuggingServerTests,
                              NonConnectingTests,
                              BadHELOServerTests, SMTPSimTests)

if __name__ == '__main__':
    test_main()
