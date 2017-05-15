/*
 * Copyright(c) 2017 Free Software Foundation, Inc.
 *
 * This file is part of Wget.
 *
 * Wget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Wget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Wget.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 * Testing IRIs
 *
 * Changelog
 * 10.09.2017  Didik Setiawan  created
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <wget.h>
#include "../libwget/private.h"

#include "../src/wget_options.h"
#include "../src/wget_log.h"

static int
	ok,
	failed;

static void test_iri_rfc(void)
{
	const struct iri_test_data {
		const char
			*uri,
			*path,
			*charset;
	} test_data[] = {
		// test reserved character based on RFC 3987 section 2.2
		// gen-delims
		{ "http://example.com/foo:bar", "foo:bar", "utf-8" },
		{ "http://example.com/foo/bar", "foo/bar", "utf-8" },
		{ "http://example.com/foo?bar", "foo?bar", "utf-8" },
		{ "http://example.com/foo#bar", "foo", "utf-8" },
		{ "http://example.com/foo[bar", "foo[bar", "utf-8" },
		{ "http://example.com/foo]bar", "foo]bar", "utf-8" },
		{ "http://example.com/foo@bar", "foo@bar", "utf-8" },
		// sub-delims
		{ "http://example.com/foo!bar", "foo!bar", "utf-8" },
		{ "http://example.com/foo$bar", "foo$bar", "utf-8" },
		{ "http://example.com/foo&bar", "foo&bar", "utf-8" },
		{ "http://example.com/foo'bar", "foo'bar", "utf-8" },
		{ "http://example.com/foo(bar", "foo(bar", "utf-8" },
		{ "http://example.com/foo)bar", "foo)bar", "utf-8" },
		{ "http://example.com/foo*bar", "foo*bar", "utf-8" },
		{ "http://example.com/foo+bar", "foo+bar", "utf-8" },
		{ "http://example.com/foo,bar", "foo,bar", "utf-8" },
		{ "http://example.com/foo;bar", "foo;bar", "utf-8" },
		{ "http://example.com/foo=bar", "foo=bar", "utf-8" }
	};

	unsigned it;

	for (it = 0; it < countof(test_data); it++) {
		const struct iri_test_data *t = &test_data[it];
		wget_iri_t *iri = wget_iri_parse(t->uri, t->charset);
		wget_http_request_t *req = wget_http_create_request(iri, "GET");

		if (wget_strcmp(req->esc_resource.data, t->path)) {
			failed++;
			printf("IRI test #%u failed:\n", it + 1);
			printf(" [%s]\n", t->uri);
			printf(" result %s (expected %s)\n", req->esc_resource.data, t->path);
			printf("\n");
		} else
			ok++;

		wget_iri_free(&iri);
		wget_http_free_request(&req);
	}
}

static void test_iri_path(void)
{
	const struct iri_test_data {
		const char
			*uri,
			*path,
			*charset;
	} test_data[] = {
		// https://github.com/cweb/url-testing/blob/master/urls-local.json
		// subsection path
		{ "http://example.com/foo/%2e", "foo/.", "utf-8"},
		{ "http://example.com/foo/%2e%2", "foo/.%2", "utf-8"},
		{ "http://example.com/foo", "foo", "utf-8"},
		{ "http://example.com/%20foo", "%20foo", "utf-8"},
		{ "http://example.com/foo%", "foo%", "utf-8"},
		{ "http://example.com/foo%2", "foo%2", "utf-8"},
		{ "http://example.com/foo%2zbar", "foo%2zbar", "utf-8"},
		{ "http://example.com/foo%41%7a", "fooAz", "utf-8"},
		{ "http://example.com/foo%00%51", "foo%00Q", "utf-8"},
		{ "http://example.com/(%28:%3A%29)", "(%28:%3A%29)", "utf-8"},
		{ "http://example.com/%3A%3a%3C%3c", "%3A%3a%3C%3c", "utf-8"},
		{ "http://example.com/%7Ffp3%3Eju%3Dduvgw%3Dd", "%7Ffp3%3Eju%3Dduvgw%3Dd", "utf-8"},
		{ "http://example.com/@asdf%40", "@asdf%40", "utf-8"},
	};

	unsigned it;

	for (it = 0; it < countof(test_data); it++) {
		const struct iri_test_data *t = &test_data[it];
		wget_iri_t *iri = wget_iri_parse(t->uri, t->charset);
		wget_http_request_t *req = wget_http_create_request(iri, "GET");

		if (wget_strcmp(req->esc_resource.data, t->path)) {
			failed++;
			printf("IRI test #%u failed:\n", it + 1);
			printf(" [%s]\n", t->uri);
			printf(" result %s (expected %s)\n", req->esc_resource.data, t->path);
			printf("\n");
		} else
			ok++;

		wget_iri_free(&iri);
		wget_http_free_request(&req);
	}
}

static void test_iri_query(void)
{
	const struct iri_test_data {
		const char
			*uri,
			*path,
			*charset;
	} test_data[] = {
		// https://github.com/cweb/url-testing/blob/master/urls-local.json
		// subsection query
		{ "http://example.com/?foo=bar", "?foo=bar", "utf-8" },
		{ "http://example.com/?as?df", "?as?df", "utf-8" },
		{ "http://example.com/?%02hello%7f bye", "?%02hello%7f%20bye", "utf-8" },
		{ "http://example.com/?%40%41123", "?%40%41123", "utf-8" },
		{ "http://example.com/?q=&lt;asdf&gt;", "?q=%3Casdf%3E", "utf-8" },
		{ "http://example.com/?q=\"asdf\"", "?q=%22asdf%22", "utf-8" }
	};

	unsigned it;

	for (it = 0; it < countof(test_data); it++) {
		const struct iri_test_data *t = &test_data[it];
		wget_iri_t *iri = wget_iri_parse(t->uri, t->charset);
		wget_http_request_t *req = wget_http_create_request(iri, "GET");

		if (wget_strcmp(req->esc_resource.data, t->path)) {
			failed++;
			printf("IRI test #%u failed:\n", it + 1);
			printf(" [%s]\n", t->uri);
			printf(" result %s (expected %s)\n", req->esc_resource.data, t->path);
			printf("\n");
		} else
			ok++;

		wget_iri_free(&iri);
		wget_http_free_request(&req);
	}
}

static void test_iri_std_url(void)
{
	const struct iri_test_data {
		const char
			*uri,
			*path,
			*charset;
	} test_data[] = {
		// https://github.com/cweb/url-testing/blob/master/urls-local.json
		// subsection standard-url
		{ "http://example.com/foo?bar=baz#", "foo?bar=baz", "utf-8" },
		{ "http://example.com/foo%2Ehtml", "foo.html", "utf-8" }
	};

	unsigned it;

	for (it = 0; it < countof(test_data); it++) {
		const struct iri_test_data *t = &test_data[it];
		wget_iri_t *iri = wget_iri_parse(t->uri, t->charset);
		wget_http_request_t *req = wget_http_create_request(iri, "GET");

		if (wget_strcmp(req->esc_resource.data, t->path)) {
			failed++;
			printf("IRI test #%u failed:\n", it + 1);
			printf(" [%s]\n", t->uri);
			printf(" result %s (expected %s)\n", req->esc_resource.data, t->path);
			printf("\n");
		} else
			ok++;

		wget_iri_free(&iri);
		wget_http_free_request(&req);
	}
}

static void test_iri_whitespace(void)
{
	const struct iri_test_data {
		const char
			*uri,
			*path,
			*charset;
	} test_data[] = {
		// https://github.com/cweb/url-testing/blob/master/urls-local.json
		// subsection whitespace
		{ "http://example.com/ ", "%20", "utf-8" },
		{ "http://example.com/foo    bar/?   foo   =   bar     #    foo",
		  "foo%20bar/?%20foo%20=%20bar", "utf-8" }
	};

	unsigned it;

	for (it = 0; it < countof(test_data); it++) {
		const struct iri_test_data *t = &test_data[it];
		wget_iri_t *iri = wget_iri_parse(t->uri, t->charset);
		wget_http_request_t *req = wget_http_create_request(iri, "GET");

		if (wget_strcmp(req->esc_resource.data, t->path)) {
			failed++;
			printf("IRI test #%u failed:\n", it + 1);
			printf(" [%s]\n", t->uri);
			printf(" result %s (expected %s)\n", req->esc_resource.data, t->path);
			printf("\n");
		} else
			ok++;

		wget_iri_free(&iri);
		wget_http_free_request(&req);
	}
}

static void test_iri_percent_enc(void)
{
	const struct iri_test_data {
		const char
			*uri,
			*path,
			*charset;
	} test_data[] = {
		// https://github.com/cweb/url-testing/blob/master/urls-local.json
		// subsection percent-encoding
		{ "http://example.com/foo%3fbar", "foo?bar", "utf-8" },
		{ "http://example.com/foo%2fbar", "foo/bar", "utf-8" },
		{ "http://example.com/%A1%C1/?foo=%EF%BD%81", "%A1%C1/?foo=%EF%BD%81", "utf-8" },
		{ "http://example.com/%A1%C1/%EF%BD%81/?foo=%A1%C1", "%A1%C1/%EF%BD%81/?foo=%A1%C1", "utf-8" },
		{ "http://example.com/%A1%C1/?foo=???", "%A1%C1/?foo=???", "utf-8" },
		{ "http://example.com/???/?foo=%A1%C1", "???/?foo=%A1%C1", "utf-8" },
		{ "http://example.com/%A1%C1/?foo=???", "%A1%C1/?foo=???", "utf-8" },
		{ "http://example.com/???/?foo=%A1%C1", "???/?foo=%A1%C1", "utf-8" },
		{ "http://example.com/D%FCrst", "D%FCrst", "utf-8" },
		{ "http://example.com/D%C3%BCrst", "D%C3%BCrst", "utf-8" },
		{ "http://example.com/?D%FCrst", "?D%FCrst", "utf-8" },
		{ "http://example.com/?D%C3%BCrst", "?D%C3%BCrst", "utf-8" }
	};

	unsigned it;

	for (it = 0; it < countof(test_data); it++) {
		const struct iri_test_data *t = &test_data[it];
		wget_iri_t *iri = wget_iri_parse(t->uri, t->charset);
		wget_http_request_t *req = wget_http_create_request(iri, "GET");

		if (wget_strcmp(req->esc_resource.data, t->path)) {
			failed++;
			printf("IRI test #%u failed:\n", it + 1);
			printf(" [%s]\n", t->uri);
			printf(" result %s (expected %s)\n", req->esc_resource.data, t->path);
			printf("\n");
		} else
			ok++;

		wget_iri_free(&iri);
		wget_http_free_request(&req);
	}
}
int main(G_GNUC_WGET_UNUSED int argc, const char **argv)
{
	// if VALGRIND testing is enabled, we have to call ourselves with
	// valgrind checking
	const char *valgrind = getenv("VALGRIND_TESTS");

	if (!valgrind || !*valgrind || !strcmp(valgrind, "0")) {
		// fallthrough
	}
	else if (!strcmp(valgrind, "1")) {
		char cmd[strlen(argv[0]) + 256];

		snprintf(cmd, sizeof(cmd), "VALGRIND_TESTS=\"\" valgrind "
				"--error-exitcode=301 --leak-check=yes "
				"--show-reachable=yes --track-origins=yes %s",
				argv[0]);
		return system(cmd) != 0;
	} else {
		char cmd[strlen(valgrind) + strlen(argv[0]) + 32];

		snprintf(cmd, sizeof(cmd), "VALGRIND_TESTS="" %s %s",
				valgrind, argv[0]);
		return system(cmd) != 0;
	}

	test_iri_rfc();
	test_iri_path();
	test_iri_query();
	test_iri_std_url();
	test_iri_whitespace();
	test_iri_percent_enc();

	//selftest_options() ? failed++ : ok++;

	if (failed) {
		printf("Summary: %d out of %d tests failed\n", failed, ok + failed);
		return 1;
	}

	printf("Summary: All %d tests passed\n", ok + failed);
	return 0;
}
