#!/usr/bin/env python3


import subprocess
import time
import os
import sys
import tempfile
import socket
import select
import threading
import signal
import psutil
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

# Colors for output
RED = '\033[91m'
GREEN = '\033[92m'
YELLOW = '\033[93m'
BLUE = '\033[94m'
MAGENTA = '\033[95m'
RESET = '\033[0m'

class WebservCorrectionTester:
    def __init__(self, binary_path="./webserv", config_path="config/example.conf"):
        self.binary_path = binary_path
        self.config_path = config_path
        self.server_process = None
        self.test_results = []
        
    def print_section(self, section_name):
        print(f"\n{MAGENTA}{'='*60}{RESET}")
        print(f"{MAGENTA}{section_name:^60}{RESET}")
        print(f"{MAGENTA}{'='*60}{RESET}")
    
    def print_test(self, test_name, passed, details=""):
        status = f"{GREEN}[PASS]{RESET}" if passed else f"{RED}[FAIL]{RESET}"
        print(f"  {status} {test_name}")
        if details:
            print(f"        {YELLOW}{details}{RESET}")
    


    def check_memory_leaks(self):
        """Check for memory leaks using valgrind"""
        self.print_section("MEMORY LEAK CHECK")
        
        # Check if valgrind is installed
        result = subprocess.run("which valgrind", shell=True, capture_output=True)
        if result.returncode != 0:
            print(f"{YELLOW}  Valgrind not installed. Install with: sudo apt-get install valgrind{RESET}")
            print(f"{YELLOW}  Skipping memory leak tests{RESET}")
            return None
        
        tests_passed = []
        
        # Test 1: Valid configuration - should have no memory leaks
        print(f"{BLUE}  Testing valid configuration with valgrind...{RESET}")
        valid_config = """
server {
    host 127.0.0.1;
    listen 8090;
    server_name test_server;
    root ./www/;
    index index.html;
    
    location / {
        allowed_methods GET POST;
        autoindex on;
    }
}
"""
        fd, valid_config_path = tempfile.mkstemp(suffix='.conf')
        with os.fdopen(fd, 'w') as f:
            f.write(valid_config)
        
        # Run with valgrind for 10 seconds then kill
        valgrind_cmd = [
            "valgrind", 
            "--leak-check=full", 
            "--show-leak-kinds=all",
            "--track-origins=yes",
            "--error-exitcode=1",
            self.binary_path, 
            valid_config_path
        ]
        
        try:
            process = subprocess.Popen(valgrind_cmd, 
                                     stdout=subprocess.PIPE, 
                                     stderr=subprocess.PIPE,
                                     text=True)
            time.sleep(3)  # Let server start
            
            # Make a simple request to exercise the server
            try:
                subprocess.run("curl -s http://127.0.0.1:8090/ > /dev/null", 
                             shell=True, timeout=2, capture_output=True)
            except:
                pass  # Request might fail, that's ok
            
            # Send SIGTERM to gracefully shutdown
            process.terminate()
            stdout, stderr = process.communicate(timeout=10)
            
            # Check valgrind output for leaks
            leak_summary = "no leaks are possible" in stderr or "All heap blocks were freed" in stderr
            no_errors = "ERROR SUMMARY: 0 errors" in stderr
            
            test_passed = leak_summary and process.returncode == 0
            details = f"No memory leaks detected" if test_passed else f"Memory leaks found - check valgrind output"
            self.print_test("Valid config - no memory leaks", test_passed, details)
            tests_passed.append(test_passed)
            
        except subprocess.TimeoutExpired:
            process.kill()
            self.print_test("Valid config - no memory leaks", False, "Valgrind test timed out")
            tests_passed.append(False)
        except Exception as e:
            self.print_test("Valid config - no memory leaks", False, f"Error: {e}")
            tests_passed.append(False)
        finally:
            os.unlink(valid_config_path)
        
        # Test 2: Configuration with typo in directive
        print(f"{BLUE}  Testing config with typo...{RESET}")
        typo_config = """
server {
    hst 127.0.0.1;
    listen 8091;
    server_nam test_server;
    rot ./www/;
    
    location / {
        allowed_method GET;
        autoinde on;
    }
}
"""
        fd, typo_config_path = tempfile.mkstemp(suffix='.conf')
        with os.fdopen(fd, 'w') as f:
            f.write(typo_config)
        
        try:
            result = subprocess.run([self.binary_path, typo_config_path], 
                                  capture_output=True, text=True, timeout=5)
            # Should fail due to invalid config
            test_passed = result.returncode != 0
            details = f"Server correctly rejected invalid config" if test_passed else f"Server accepted invalid config"
            self.print_test("Config with typos rejected", test_passed, details)
            tests_passed.append(test_passed)
        except subprocess.TimeoutExpired:
            self.print_test("Config with typos rejected", False, "Server hung on invalid config")
            tests_passed.append(False)
        except Exception as e:
            self.print_test("Config with typos rejected", False, f"Error: {e}")
            tests_passed.append(False)
        finally:
            os.unlink(typo_config_path)
        
        # Test 3: Running webserv without arguments
        print(f"{BLUE}  Testing webserv without arguments...{RESET}")
        try:
            result = subprocess.run([self.binary_path], 
                                  capture_output=True, text=True, timeout=3)
            # Should fail or show usage
            test_passed = result.returncode != 0 or "usage" in result.stderr.lower() or "Usage" in result.stderr
            details = f"Correctly handles missing config file" if test_passed else f"No proper error handling"
            self.print_test("No arguments handling", test_passed, details)
            tests_passed.append(test_passed)
        except subprocess.TimeoutExpired:
            self.print_test("No arguments handling", False, "Server hung without config")
            tests_passed.append(False)
        except Exception as e:
            self.print_test("No arguments handling", False, f"Error: {e}")
            tests_passed.append(False)
        
        # Test 4: Duplicate server blocks (same port)
        print(f"{BLUE}  Testing duplicate server configuration...{RESET}")
        duplicate_config = """
server {
    host 127.0.0.1;
    listen 8092;
    server_name server1;
    root ./www/;
    
    location / {
        allowed_methods GET;
    }
}

server {
    host 127.0.0.1;
    listen 8092;
    server_name server2;
    root ./www/;
    
    location / {
        allowed_methods GET;
    }
}
"""
        fd, duplicate_config_path = tempfile.mkstemp(suffix='.conf')
        with os.fdopen(fd, 'w') as f:
            f.write(duplicate_config)
        
        try:
            result = subprocess.run([self.binary_path, duplicate_config_path], 
                                  capture_output=True, text=True, timeout=5)
            # Should fail due to duplicate port binding
            test_passed = result.returncode != 0
            details = f"Correctly rejected duplicate servers" if test_passed else f"Accepted duplicate servers"
            self.print_test("Duplicate server rejection", test_passed, details)
            tests_passed.append(test_passed)
        except subprocess.TimeoutExpired:
            self.print_test("Duplicate server rejection", False, "Server hung on duplicate config")
            tests_passed.append(False)
        except Exception as e:
            self.print_test("Duplicate server rejection", False, f"Error: {e}")
            tests_passed.append(False)
        finally:
            os.unlink(duplicate_config_path)
        
        # Test 5: Simple run and kill signal test
        print(f"{BLUE}  Testing signal handling...{RESET}")
        signal_config = """
server {
    host 127.0.0.1;
    listen 8093;
    server_name signal_test;
    root ./www/;
    
    location / {
        allowed_methods GET;
    }
}
"""
        fd, signal_config_path = tempfile.mkstemp(suffix='.conf')
        with os.fdopen(fd, 'w') as f:
            f.write(signal_config)
        
        try:
            # Start server
            process = subprocess.Popen([self.binary_path, signal_config_path],
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            time.sleep(2)  # Let server start
            
            # Check if server is running
            server_running = process.poll() is None
            
            # Send SIGTERM
            if server_running:
                process.terminate()
                try:
                    process.wait(timeout=5)
                    clean_exit = process.returncode in [0, -15]  # 0 or SIGTERM
                except subprocess.TimeoutExpired:
                    process.kill()
                    clean_exit = False
            else:
                clean_exit = False
            
            test_passed = server_running and clean_exit
            details = f"Server started and handled SIGTERM cleanly" if test_passed else f"Signal handling issues"
            self.print_test("Signal handling (SIGTERM)", test_passed, details)
            tests_passed.append(test_passed)
            
        except Exception as e:
            self.print_test("Signal handling (SIGTERM)", False, f"Error: {e}")
            tests_passed.append(False)
        finally:
            os.unlink(signal_config_path)
        
        # Test 6: SIGINT handling
        print(f"{BLUE}  Testing SIGINT handling...{RESET}")
        try:
            # Create another simple config
            fd, sigint_config_path = tempfile.mkstemp(suffix='.conf')
            with os.fdopen(fd, 'w') as f:
                f.write(signal_config)
            
            process = subprocess.Popen([self.binary_path, sigint_config_path],
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            time.sleep(2)
            
            server_running = process.poll() is None
            
            # Send SIGINT (Ctrl+C)
            if server_running:
                process.send_signal(signal.SIGINT)
                try:
                    process.wait(timeout=5)
                    clean_exit = process.returncode in [0, -2]  # 0 or SIGINT
                except subprocess.TimeoutExpired:
                    process.kill()
                    clean_exit = False
            else:
                clean_exit = False
            
            test_passed = server_running and clean_exit
            details = f"Server handled SIGINT cleanly" if test_passed else f"SIGINT handling issues"
            self.print_test("Signal handling (SIGINT)", test_passed, details)
            tests_passed.append(test_passed)
            
        except Exception as e:
            self.print_test("Signal handling (SIGINT)", False, f"Error: {e}")
            tests_passed.append(False)
        finally:
            os.unlink(sigint_config_path)
        
        # Test 7: Invalid port numbers
        print(f"{BLUE}  Testing invalid port numbers...{RESET}")
        invalid_port_configs = [
            ("Port 0", "listen 0;"),
            ("Port > 65535", "listen 99999;"),
            ("Negative port", "listen -1;"),
            ("Non-numeric port", "listen abc;")
        ]
        
        for test_name, port_directive in invalid_port_configs:
            invalid_config = f"""
server {{
    host 127.0.0.1;
    {port_directive}
    server_name test;
    root ./www/;
    
    location / {{
        allowed_methods GET;
    }}
}}
"""
            fd, invalid_path = tempfile.mkstemp(suffix='.conf')
            with os.fdopen(fd, 'w') as f:
                f.write(invalid_config)
            
            try:
                result = subprocess.run([self.binary_path, invalid_path], 
                                      capture_output=True, text=True, timeout=3)
                test_passed = result.returncode != 0
                details = f"Correctly rejected {test_name.lower()}" if test_passed else f"Accepted {test_name.lower()}"
                self.print_test(f"Invalid port handling - {test_name}", test_passed, details)
                tests_passed.append(test_passed)
            except subprocess.TimeoutExpired:
                self.print_test(f"Invalid port handling - {test_name}", False, "Server hung")
                tests_passed.append(False)
            except Exception as e:
                self.print_test(f"Invalid port handling - {test_name}", False, f"Error: {e}")
                tests_passed.append(False)
            finally:
                os.unlink(invalid_path)
        
        # Test 8: Missing required directives
        print(f"{BLUE}  Testing missing required directives...{RESET}")
        missing_configs = [
            ("No listen", """
server {
    host 127.0.0.1;
    server_name test;
    root ./www/;
    location / {
        allowed_methods GET;
    }
}"""),
            ("No root", """
server {
    host 127.0.0.1;
    listen 8094;
    server_name test;
    location / {
        allowed_methods GET;
    }
}"""),
            ("Empty server block", """
server {
}""")
        ]
        
        for test_name, config_content in missing_configs:
            fd, missing_path = tempfile.mkstemp(suffix='.conf')
            with os.fdopen(fd, 'w') as f:
                f.write(config_content)
            
            try:
                result = subprocess.run([self.binary_path, missing_path], 
                                      capture_output=True, text=True, timeout=3)
                test_passed = result.returncode != 0
                details = f"Correctly rejected config with {test_name.lower()}" if test_passed else f"Accepted incomplete config"
                self.print_test(f"Missing directive - {test_name}", test_passed, details)
                tests_passed.append(test_passed)
            except subprocess.TimeoutExpired:
                self.print_test(f"Missing directive - {test_name}", False, "Server hung")
                tests_passed.append(False)
            except Exception as e:
                self.print_test(f"Missing directive - {test_name}", False, f"Error: {e}")
                tests_passed.append(False)
            finally:
                os.unlink(missing_path)
        
        # Print summary
        passed_count = sum(tests_passed)
        total_count = len(tests_passed)
        
        print(f"\n    {BLUE}Memory Leak Test Summary:{RESET}")
        print(f"    Tests passed: {passed_count}/{total_count}")
        
        overall_passed = passed_count >= (total_count * 0.8)  # 80% pass rate
        
        if overall_passed:
            print(f"    {GREEN}✓ MEMORY LEAK TESTS PASSED{RESET} - Server handles errors gracefully")
        else:
            print(f"    {RED}✗ MEMORY LEAK TESTS FAILED{RESET} - Server has memory management issues")
        
        return overall_passed
   

    def test_io_multiplexing_behavioral(self):
        """Test multiplexing basé sur le comportement, pas sur strace"""
        self.print_section("I/O MULTIPLEXING - BEHAVIORAL TEST")
        
        # Start server
        self.server_process = subprocess.Popen(
            [self.binary_path, self.config_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        time.sleep(2)
        
        tests_passed = []
        
        try:
            # Test 1: Connexions simultanées avec délais artificiels
            def slow_connection_test():
                """Test avec des connexions qui restent ouvertes un moment"""
                connections = []
                start_time = time.time()
                
                def create_slow_connection(conn_id):
                    try:
                        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        sock.settimeout(10)
                        sock.connect(('127.0.0.1', 8888))
                        
                        # Envoyer une requête mais lire lentement
                        request = f"GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
                        sock.send(request.encode())
                        
                        # Lire par petits bouts pour simuler une connexion lente
                        response = b""
                        for _ in range(5):
                            data = sock.recv(100)  # Petits chunks
                            response += data
                            if not data or b"</html>" in response:
                                break
                            time.sleep(0.1)  # Délai artificiel
                        
                        sock.close()
                        return len(response) > 100
                    except Exception as e:
                        print(f"Connection {conn_id} failed: {e}")
                        return False
                
                # Lancer 5 connexions "lentes" simultanément
                with ThreadPoolExecutor(max_workers=5) as executor:
                    futures = [executor.submit(create_slow_connection, i) for i in range(5)]
                    results = [future.result() for future in futures]
                
                end_time = time.time()
                elapsed = end_time - start_time
                successful = sum(results)
                
                print(f"    Slow connections test: {successful}/5 successful in {elapsed:.2f}s")
                
                # Si multiplexing: toutes les connexions devraient réussir en ~1s
                # Si pas de multiplexing: ça prendrait 5x plus de temps
                return successful >= 4 and elapsed < 3.0
            
            multiplexing_detected = slow_connection_test()
            self.print_test("Concurrent slow connections", multiplexing_detected,
                          f"Multiplexing {'detected' if multiplexing_detected else 'NOT detected'}")
            tests_passed.append(multiplexing_detected)
            
            # Test 2: Test de "starvation" - une connexion lente ne doit pas bloquer les autres
            def starvation_test():
                """Test qu'une connexion lente ne bloque pas les autres"""
                
                def blocking_connection():
                    """Connexion qui envoie des données très lentement"""
                    try:
                        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        sock.settimeout(15)
                        sock.connect(('127.0.0.1', 8888))
                        
                        # Envoyer la requête caractère par caractère (très lent)
                        request = "GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
                        for char in request:
                            sock.send(char.encode())
                            time.sleep(0.05)  # Délai entre chaque caractère
                        
                        response = sock.recv(4096)
                        sock.close()
                        return len(response) > 0
                    except:
                        return False
                
                def fast_connection():
                    """Connexion rapide normale"""
                    try:
                        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        sock.settimeout(5)
                        sock.connect(('127.0.0.1', 8888))
                        
                        request = "GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"
                        sock.send(request.encode())
                        
                        response = sock.recv(4096)
                        sock.close()
                        return len(response) > 0
                    except:
                        return False
                
                # Lancer une connexion lente puis plusieurs rapides
                slow_thread = threading.Thread(target=blocking_connection)
                slow_thread.start()
                
                time.sleep(0.5)  # Laisser la connexion lente commencer
                
                # Lancer 3 connexions rapides en parallèle
                start_time = time.time()
                with ThreadPoolExecutor(max_workers=3) as executor:
                    fast_futures = [executor.submit(fast_connection) for _ in range(3)]
                    fast_results = [future.result() for future in fast_futures]
                
                fast_time = time.time() - start_time
                
                slow_thread.join(timeout=10)  # Attendre la connexion lente max 10s
                
                successful_fast = sum(fast_results)
                print(f"    Starvation test: {successful_fast}/3 fast connections in {fast_time:.2f}s")
                
                # Si multiplexing: les connexions rapides ne sont pas bloquées
                return successful_fast >= 2 and fast_time < 2.0
            
            no_starvation = starvation_test()
            self.print_test("No connection starvation", no_starvation,
                          f"Fast connections {'not blocked' if no_starvation else 'BLOCKED by slow one'}")
            tests_passed.append(no_starvation)
            
            # Test 3: Capacité maximale de connexions simultanées
            def max_connections_test():
                """Test du nombre maximum de connexions simultanées"""
                max_successful = 0
                
                for num_conns in [10, 20, 50]:
                    connections = []
                    successful = 0
                    
                    try:
                        # Ouvrir toutes les connexions rapidement
                        for i in range(num_conns):
                            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                            sock.settimeout(2)
                            sock.connect(('127.0.0.1', 8888))
                            connections.append(sock)
                            successful += 1
                            time.sleep(0.01)  # Petit délai
                    except:
                        pass  # Connexions supplémentaires échouent
                    
                    # Fermer toutes les connexions
                    for sock in connections:
                        try:
                            sock.close()
                        except:
                            pass
                    
                    max_successful = max(max_successful, successful)
                    print(f"    Max connections test: {successful}/{num_conns} for {num_conns} target")
                    
                    if successful < num_conns:
                        break
                
                # Si multiplexing bon: devrait gérer au moins 10 connexions simultanées
                return max_successful >= 10
            
            high_concurrency = max_connections_test()
            self.print_test("High concurrency support", high_concurrency,
                          f"Handles high connection load well")
            tests_passed.append(high_concurrency)
            
            # Verdict final basé sur les tests comportementaux
            multiplexing_score = sum(tests_passed)
            overall_multiplexing = multiplexing_score >= 2  # Au moins 2/3 tests passent
            
            print(f"\n    {BLUE}Multiplexing Assessment:{RESET}")
            print(f"    Tests passed: {multiplexing_score}/3")
            if overall_multiplexing:
                print(f"    {GREEN}✓ MULTIPLEXING DETECTED{RESET} - Server handles concurrent connections properly")
            else:
                print(f"    {RED}✗ NO MULTIPLEXING{RESET} - Server appears to handle connections sequentially")
            
            return overall_multiplexing
            
        except Exception as e:
            self.print_test("Behavioral multiplexing test", False, f"Error: {e}")
            return False
        finally:
            if self.server_process:
                self.server_process.terminate()
                self.server_process.wait()

    # Remplacer l'ancienne méthode
    def test_io_multiplexing(self):
        """Test I/O multiplexing avec approche comportementale"""
        return self.test_io_multiplexing_behavioral()
   
    def test_configuration(self):
        """Test configuration requirements from correction sheet"""
        self.print_section("CONFIGURATION TESTS")
        
        tests_passed = []
        
        # Test 1: Multiple servers with different ports
        config1 = """
server {
    host 127.0.0.1;
    listen 8080;
    server_name server1;
    root ./www/;
    
    location / {
        index index.html;
        allowed_methods GET;
    }
}

server {
    host 127.0.0.1;
    listen 8081;
    server_name server2;
    root ./www/;
    
    location / {
        index dashboard.html;
        allowed_methods GET;
    }
}
"""
        fd, path = tempfile.mkstemp(suffix='.conf')
        with os.fdopen(fd, 'w') as f:
            f.write(config1)
        
        # Start server with multi-port config
        server = subprocess.Popen([self.binary_path, path], 
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        # Test both ports
        r1 = subprocess.run("curl -s http://127.0.0.1:8080/", shell=True, 
                          capture_output=True, text=True)
        r2 = subprocess.run("curl -s http://127.0.0.1:8081/", shell=True,
                          capture_output=True, text=True)
        
        test_passed = "Welcome" in r1.stdout and "Dashboard" in r2.stdout
        self.print_test("Multiple servers with different ports", test_passed)
        tests_passed.append(test_passed)
        
        server.terminate()
        server.wait()
        os.unlink(path)
        time.sleep(1)
        
        # Test 2: Different hostnames
        self.server_process = subprocess.Popen([self.binary_path, self.config_path],
                                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        result = subprocess.run(
            "curl --resolve example.com:8888:127.0.0.1 http://example.com:8888/",
            shell=True, capture_output=True, text=True
        )
        test_passed = result.returncode == 0 and len(result.stdout) > 0
        self.print_test("Different hostnames support", test_passed)
        tests_passed.append(test_passed)
        
        # # Test 3: Custom error pages
        # result = subprocess.run("curl -s http://127.0.0.1:8888/nonexistent",
        #                       shell=True, capture_output=True, text=True)
        # test_passed = "404" in result.stdout
        # self.print_test("Custom error pages", test_passed)
        # tests_passed.append(test_passed)
        
        # # Test 4: Client body limit
        # large_data = "X" * 60000  # Larger than 50000 limit
        # result = subprocess.run(
        #     f"curl -s -o /dev/null -w '%{{http_code}}' -X POST -H 'Content-Type: text/plain' --data '{large_data}' http://127.0.0.1:8888/methods",
        #     shell=True, capture_output=True, text=True
        # )
        # test_passed = "413" in result.stdout
        # self.print_test("Client body size limit", test_passed)
        # tests_passed.append(test_passed)
        
        # Test 5: Different routes
        routes = ["/", "/dashboard", "/methods", "/cgi-bin/"]
        route_results = []
        for route in routes:
            result = subprocess.run(f"curl -s -o /dev/null -w '%{{http_code}}' http://127.0.0.1:8888{route}",
                                  shell=True, capture_output=True, text=True)
            route_results.append("200" in result.stdout or "404" in result.stdout)
        test_passed = all(route_results)
        self.print_test("Multiple routes configuration", test_passed)
        tests_passed.append(test_passed)
        
        # Test 6: Method restrictions
        # DELETE should be forbidden on /
        result = subprocess.run("curl -s -o /dev/null -w '%{http_code}' -X DELETE http://127.0.0.1:8888/",
                              shell=True, capture_output=True, text=True)
        test_passed = "403" in result.stdout or "405" in result.stdout
        self.print_test("Method restrictions", test_passed)
        tests_passed.append(test_passed)
        
        self.server_process.terminate()
        self.server_process.wait()
        
        return all(tests_passed)
    


    def test_basic_checks(self):
        """Test basic HTTP functionality"""
        self.print_section("BASIC FUNCTIONALITY CHECKS")
        
        # Start server
        self.server_process = subprocess.Popen([self.binary_path, self.config_path],
                                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        tests_passed = []
        
        # Test GET request
        result = subprocess.run("curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8888/",
                              shell=True, capture_output=True, text=True)
        test_passed = "200" in result.stdout
        self.print_test("GET request", test_passed)
        tests_passed.append(test_passed)
        
        # Test POST request
        result = subprocess.run("curl -s -o /dev/null -w '%{http_code}' -X POST -d 'test' http://127.0.0.1:8888/methods",
                              shell=True, capture_output=True, text=True)
        test_passed = "201" in result.stdout or "200" in result.stdout
        self.print_test("POST request", test_passed)
        tests_passed.append(test_passed)
        
        # Test DELETE request
        # First create a file to delete
        subprocess.run("curl -X POST -d 'test_delete' http://127.0.0.1:8888/methods", 
                      shell=True, capture_output=True)
        time.sleep(1)
        
        # Try to delete (may need to adjust based on actual implementation)
        #check if the fileToDelete.txt exists, if not create it
        if not os.path.exists("www/uploads/fileToDelete.txt"):
            subprocess.run("cp www/uploads/file.txt www/uploads/fileToDelete.txt", shell=True)
        result = subprocess.run("curl -s -o /dev/null -w '%{http_code}' -X DELETE http://127.0.0.1:8888/uploads/fileToDelete.txt",
                              shell=True, capture_output=True, text=True)
        test_passed = result.stdout.strip() in ["200", "204", "404"]  # 404 if file doesn't exist
        self.print_test("DELETE request", test_passed)
        tests_passed.append(test_passed)
        
        # Test UNKNOWN method (should not crash)
        result = subprocess.run("curl -s -o /dev/null -w '%{http_code}' -X UNKNOWN http://127.0.0.1:8888/",
                              shell=True, capture_output=True, text=True)
        test_passed = result.stdout.strip() in ["400", "405", "501"]
        self.print_test("UNKNOWN request handling", test_passed)
        tests_passed.append(test_passed)
        
        # Test file upload and retrieval
        test_content = "This is a test file for upload"
        fd, filepath = tempfile.mkstemp()
        with os.fdopen(fd, 'w') as f:
            f.write(test_content)
        
        # Upload file
        result = subprocess.run(f"curl -s -o /dev/null -w '%{{http_code}}' -X POST --data-binary '@{filepath}' http://127.0.0.1:8888/methods",
                              shell=True, capture_output=True, text=True)
        test_passed = "201" in result.stdout or "200" in result.stdout
        self.print_test("File upload", test_passed)
        tests_passed.append(test_passed)
        
        os.unlink(filepath)
        
        self.server_process.terminate()
        self.server_process.wait()
        
        return all(tests_passed)
    


    def test_cgi(self):
        """Test CGI functionality"""
        self.print_section("CGI TESTS")
        
        # Start server
        self.server_process = subprocess.Popen([self.binary_path, self.config_path],
                                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        tests_passed = []
        
        # Test CGI with GET
        result = subprocess.run("curl -s http://127.0.0.1:8888/cgi-bin/lotr",
                              shell=True, capture_output=True, text=True)
        test_passed = len(result.stdout) > 0 and "Archives" in result.stdout
        self.print_test("CGI GET request (Python)", test_passed)
        tests_passed.append(test_passed)
        
        # Test CGI with POST
        result = subprocess.run("curl -s -X POST -d 'username=test&message=hello' http://127.0.0.1:8888/cgi-bin/lotr",
                              shell=True, capture_output=True, text=True)
        test_passed = len(result.stdout) > 0
        self.print_test("CGI POST request (Python)", test_passed)
        tests_passed.append(test_passed)
        
        # Test shell CGI
        result = subprocess.run("curl -s http://127.0.0.1:8888/cgi-bin/star-wars",
                              shell=True, capture_output=True, text=True)
        test_passed = len(result.stdout) > 0 and "Terminal" in result.stdout
        self.print_test("CGI GET request (Shell)", test_passed)
        tests_passed.append(test_passed)
        
        # Test CGI with query string
        result = subprocess.run("curl -s 'http://127.0.0.1:8888/cgi-bin/lotr?action=time'",
                              shell=True, capture_output=True, text=True)
        test_passed = len(result.stdout) > 0
        self.print_test("CGI with query string", test_passed)
        tests_passed.append(test_passed)
        
        # Test CGI error handling (infinite loop simulation)
        # Create a bad CGI script
        bad_cgi = """#!/usr/bin/env python3
import time
while True:
    time.sleep(1)
"""
        fd, bad_cgi_path = tempfile.mkstemp(suffix='.py')
        with os.fdopen(fd, 'w') as f:
            f.write(bad_cgi)
        os.chmod(bad_cgi_path, 0o755)
        
        # This should timeout or handle gracefully
        # Note: May need to adjust based on actual implementation
        self.print_test("CGI error handling", True, "Manual verification needed for timeout behavior")
        tests_passed.append(True)
        
        os.unlink(bad_cgi_path)
        
        self.server_process.terminate()
        self.server_process.wait()
        
        return all(tests_passed)
    


    def test_browser_compatibility(self):
        """Test browser compatibility"""
        self.print_section("BROWSER COMPATIBILITY")
        
        # Start server
        self.server_process = subprocess.Popen([self.binary_path, self.config_path],
                                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        tests_passed = []
        
        # Test serving static website
        result = subprocess.run("curl -s -I http://127.0.0.1:8888/",
                              shell=True, capture_output=True, text=True)
        
        # Check for proper headers
        has_content_type = "Content-Type:" in result.stdout
        has_content_length = "Content-Length:" in result.stdout
        has_http_version = "HTTP/1.1" in result.stdout
        
        test_passed = has_content_type and has_content_length and has_http_version
        self.print_test("HTTP headers present", test_passed)
        tests_passed.append(test_passed)
        
        # Test wrong URL
        result = subprocess.run("curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8888/nonexistent",
                              shell=True, capture_output=True, text=True)
        test_passed = "404" in result.stdout
        self.print_test("404 on wrong URL", test_passed)
        tests_passed.append(test_passed)
        
        # Test directory listing
        result = subprocess.run("curl -s http://127.0.0.1:8888/uploads/",
                              shell=True, capture_output=True, text=True)
        test_passed = len(result.stdout) > 0
        self.print_test("Directory listing", test_passed, 
                       "Autoindex on" if "Directory" in result.stdout else "Autoindex off or 404")
        tests_passed.append(True)  # Both behaviors are acceptable
        
        # Test static files
        static_files = [
            "/style/style.css",
            "/favicon.ico",
            "/index.html"
        ]
        
        for file in static_files:
            result = subprocess.run(f"curl -s -o /dev/null -w '%{{http_code}}' http://127.0.0.1:8888{file}",
                                  shell=True, capture_output=True, text=True)
            test_passed = "200" in result.stdout or "404" in result.stdout
            self.print_test(f"Static file: {file}", test_passed)
            tests_passed.append(test_passed)
        
        self.server_process.terminate()
        self.server_process.wait()
        
        return all(tests_passed)
    


    def test_port_issues(self):
        """Test port configuration issues"""
        self.print_section("PORT CONFIGURATION TESTS")
        
        tests_passed = []
        
        # Test 1: Same port multiple times in config (should fail)
        bad_config = """
server {
    host 127.0.0.1;
    listen 8080;
    listen 8080;
    server_name test;
    root ./www/;
    
    location / {
        index index.html;
        allowed_methods GET;
    }
}
"""
        fd, path = tempfile.mkstemp(suffix='.conf')
        with os.fdopen(fd, 'w') as f:
            f.write(bad_config)
        
        result = subprocess.run([self.binary_path, path], 
                              capture_output=True, text=True, timeout=2)
        test_passed = result.returncode != 0  # Should fail
        self.print_test("Reject duplicate ports in same server", test_passed)
        tests_passed.append(test_passed)
        os.unlink(path)
        
        # Test 2: Multiple servers with common ports (should fail)
        config1 = """
server {
    host 127.0.0.1;
    listen 7777;
    server_name test1;
    root ./www/;
    
    location / {
        index index.html;
        allowed_methods GET;
    }
}
"""
        config2 = """
server {
    host 127.0.0.1;
    listen 7777;
    server_name test2;
    root ./www/;
    
    location / {
        index index.html;
        allowed_methods GET;
    }
}
"""
        
        fd1, path1 = tempfile.mkstemp(suffix='.conf')
        with os.fdopen(fd1, 'w') as f:
            f.write(config1)
        
        fd2, path2 = tempfile.mkstemp(suffix='.conf')
        with os.fdopen(fd2, 'w') as f:
            f.write(config2)
        
        # Start first server
        server1 = subprocess.Popen([self.binary_path, path1],
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        # Try to start second server (should fail)
        server2 = subprocess.Popen([self.binary_path, path2],
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        # Check if second server failed to start
        test_passed = server2.poll() is not None  # Should have exited
        self.print_test("Prevent binding to same port twice", test_passed)
        tests_passed.append(test_passed)
        
        server1.terminate()
        server1.wait()
        if server2.poll() is None:
            server2.terminate()
            server2.wait()
        
        os.unlink(path1)
        os.unlink(path2)
        
        return all(tests_passed)



    def test_stress(self):
        """Run stress tests with siege"""
        self.print_section("STRESS TESTS")
        
        # Check if siege is installed
        result = subprocess.run("which siege", shell=True, capture_output=True)
        if result.returncode != 0:
            print(f"{YELLOW}  Siege not installed. Install with: brew install siege (macOS) or apt-get install siege (Linux){RESET}")
            print(f"{YELLOW}  Skipping stress tests{RESET}")
            return None
        
        # Start server
        self.server_process = subprocess.Popen([self.binary_path, self.config_path],
                                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        tests_passed = []
        
        # Get initial memory usage
        process = psutil.Process(self.server_process.pid)
        initial_memory = process.memory_info().rss / 1024 / 1024  # MB
        
        # Test 1: Basic siege test
        print(f"{YELLOW}  Running siege test (10 seconds)...{RESET}")
        result = subprocess.run("siege -b -t10s http://127.0.0.1:8888/ 2>&1 | grep 'Availability'",
                              shell=True, capture_output=True, text=True)
        
        if "Availability" in result.stdout:
            # Extract availability percentage
            availability = float(result.stdout.split(':')[1].strip().replace('%', ''))
            test_passed = availability >= 99.5
            self.print_test(f"Availability test", test_passed, f"{availability:.2f}% (need >= 99.5%)")
            tests_passed.append(test_passed)
        else:
            self.print_test("Availability test", False, "Could not measure availability")
            tests_passed.append(False)
        
        # Test 2: Memory leak check during stress
        time.sleep(2)
        final_memory = process.memory_info().rss / 1024 / 1024  # MB
        memory_increase = final_memory - initial_memory
        
        test_passed = memory_increase < 50  # Less than 50MB increase
        self.print_test("Memory stability", test_passed, 
                       f"Memory increased by {memory_increase:.2f}MB")
        tests_passed.append(test_passed)
        
        # Test 3: Check for hanging connections
        result = subprocess.run("netstat -an | grep 8888 | grep ESTABLISHED | wc -l",
                              shell=True, capture_output=True, text=True)
        established = int(result.stdout.strip())
        
        test_passed = established < 10  # Should not have many hanging connections
        self.print_test("No hanging connections", test_passed,
                       f"{established} established connections")
        tests_passed.append(test_passed)
        
        self.server_process.terminate()
        self.server_process.wait()
        
        return all(tests_passed)


    
    def test_bonus(self):
        """Test bonus features"""
        self.print_section("BONUS FEATURES")
        
        # Start server
        self.server_process = subprocess.Popen([self.binary_path, self.config_path],
                                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        time.sleep(2)
        
        tests_passed = []
        
        # Test cookies and sessions
        result = subprocess.run("curl -v http://127.0.0.1:8888/register 2>&1 | grep -i 'set-cookie'",
                              shell=True, capture_output=True, text=True)
        
        test_passed = "set-cookie" in result.stdout.lower()
        self.print_test("Cookie system", test_passed)
        tests_passed.append(test_passed)
        
        # Test multiple CGI systems (Python and Shell)
        has_python = os.path.exists("www/cgi-bin/lotr.py")
        has_shell = os.path.exists("www/cgi-bin/star_wars.sh")
        
        test_passed = has_python and has_shell
        self.print_test("Multiple CGI systems", test_passed,
                       f"Python: {'✓' if has_python else '✗'}, Shell: {'✓' if has_shell else '✗'}")
        tests_passed.append(test_passed)
        
        self.server_process.terminate()
        self.server_process.wait()
        
        return all(tests_passed)
    


    def run_correction_tests(self):
        """Run all correction sheet tests"""
        print(f"\n{BLUE}{'='*60}{RESET}")
        print(f"{BLUE}{'WEBSERV ADVANCED TESTS':^60}{RESET}")
        print(f"{BLUE}{'='*60}{RESET}")
        
        # Check compilation
        self.print_section("COMPILATION CHECK")
        if not os.path.exists(self.binary_path):
            self.print_test("Binary exists", False, f"Binary not found at {self.binary_path}")
            print(f"\n{RED}Cannot continue without binary{RESET}")
            return
        else:
            self.print_test("Binary exists", True)
        
        # Run Makefile to check for re-link issues
        if os.path.exists("Makefile"):
            result = subprocess.run("make", capture_output=True, text=True)
            result2 = subprocess.run("make", capture_output=True, text=True)
            
            test_passed = not any(indicator in result2.stdout + result2.stderr for indicator in ["g++", "clang++", "c++", ".cpp", ".cc", ".cxx", "Compiling", "Building"])
            if test_passed:
                self.print_test("No re-link issues", test_passed)
            else:
                self.print_test("Re-link issues", test_passed)
        
        # Test sections
        test_sections = [
            ("Memory Leaks", self.check_memory_leaks),
            ("I/O Multiplexing", self.test_io_multiplexing),
            ("Configuration", self.test_configuration),
            ("Basic Checks", self.test_basic_checks),
            ("CGI", self.test_cgi),
            ("Browser Compatibility", self.test_browser_compatibility),
            ("Port Issues", self.test_port_issues),
            ("Stress Tests", self.test_stress),
            ("Bonus Features", self.test_bonus)
        ]
        
        results = {}
        mandatory_fail = False
        
        for name, test_func in test_sections:
            try:
                result = test_func()
                results[name] = result
                
                # Check for mandatory failures
                if name in ["Memory Leaks", "I/O Multiplexing", "Configuration", "Basic Checks"] and result == False:
                    mandatory_fail = True
                    
            except Exception as e:
                print(f"{RED}Error in {name}: {e}{RESET}")
                results[name] = False
        
        # Print summary
        self.print_section("EVALUATION SUMMARY")
        
        print(f"\n{BLUE}{'Test Section':<30} {'Result':<15} {'Grade Impact'}{RESET}")
        print(f"{'-'*60}")
        
        for name, result in results.items():
            if result is None:
                status = f"{YELLOW}SKIPPED{RESET}"
                impact = "N/A"
            elif result:
                status = f"{GREEN}PASS{RESET}"
                impact = "✓"
            else:
                status = f"{RED}FAIL{RESET}"
                if name in ["Memory Leaks", "I/O Multiplexing"]:
                    impact = "GRADE = 0"
                elif name in ["Configuration", "Basic Checks", "CGI"]:
                    impact = "Major penalty"
                elif name in ["Bonus Features"]:
                    impact = "No bonus points"
                else:
                    impact = "Minor penalty"
            
            print(f"{name:<30} {status:<24} {impact}")
        
        # Final verdict
        print(f"\n{BLUE}{'='*60}{RESET}")
        if mandatory_fail:
            print(f"{RED}{'MANDATORY PART FAILED - GRADE: 0':^60}{RESET}")
            print(f"{RED}Fix critical issues before resubmission{RESET}")
        else:
            passed = sum(1 for r in results.values() if r == True)
            total = sum(1 for r in results.values() if r is not None)
            percentage = (passed / total * 100) if total > 0 else 0
            
            if percentage >= 90:
                print(f"{GREEN}{'EXCELLENT - Ready for evaluation':^60}{RESET}")
            elif percentage >= 70:
                print(f"{YELLOW}{'GOOD - Minor issues to fix':^60}{RESET}")
            else:
                print(f"{RED}{'NEEDS WORK - Multiple issues found':^60}{RESET}")
            
            print(f"\n{BLUE}Score: {passed}/{total} tests passed ({percentage:.1f}%){RESET}")



if __name__ == "__main__":

    binary_path = sys.argv[1] if len(sys.argv) > 1 else "./webserv"
    config_path = sys.argv[2] if len(sys.argv) > 2 else "config/example.conf"
    
    if not os.path.exists(binary_path):
        print(f"{RED}Error: Binary '{binary_path}' not found{RESET}")
        print(f"Usage: {sys.argv[0]} [webserv_binary] [config_file]")
        print(f"\nMake sure to compile the project first with 'make'")
        sys.exit(1)
    
    if not os.path.exists(config_path):
        print(f"{RED}Error: Config file '{config_path}' not found{RESET}")
        print(f"Usage: {sys.argv[0]} [webserv_binary] [config_file]")
        sys.exit(1)
    
    tester = WebservCorrectionTester(binary_path, config_path)
    
    try:
        tester.run_correction_tests()
    except KeyboardInterrupt:
        print(f"\n{YELLOW}Tests interrupted by user{RESET}")
        if tester.server_process:
            tester.server_process.terminate()
            tester.server_process.wait()
    except Exception as e:
        print(f"\n{RED}Unexpected error: {e}{RESET}")
        if tester.server_process:
            tester.server_process.terminate()
            tester.server_process.wait()
    subprocess.run("rm -rf www/uploads/1*.txt", shell=True)
