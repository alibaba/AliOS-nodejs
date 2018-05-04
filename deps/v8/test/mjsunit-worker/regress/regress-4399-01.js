// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Test that switch has the appropriate 'eval' value

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4399-01.js");
	}
}
assertEquals("foo", eval('switch(1) { case 1: "foo" }'));
assertEquals("foo", eval('{ switch(1) { case 1: "foo" } }'));
assertEquals("foo", eval('switch(1) { case 1: { "foo" } }'));
assertEquals("foo", eval('switch(1) { case 1: "foo"; break; case 2: "bar"; break }'));
assertEquals("bar", eval('switch(2) { case 1: "foo"; break; case 2: "bar"; break }'));
assertEquals("bar", eval('switch(1) { case 1: "foo"; case 2: "bar"; break }'));

// The tag is not the value, if there's no value

assertEquals(undefined, eval('switch (1) {}'));
assertEquals(undefined, eval('switch (1) { case 1: {} }'));
