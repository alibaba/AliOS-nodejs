// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Some surrounding cases which already worked, for good measure
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4640.js");
	}
}
assertTrue(new Date('275760-10-14') == 'Invalid Date');
assertTrue(new Date('275760-09-23') == 'Invalid Date');
assertTrue(new Date('+275760-09-24') == 'Invalid Date');
assertTrue(new Date('+275760-10-13') == 'Invalid Date');

// The following cases used to throw "illegal access"
assertTrue(new Date('275760-09-24') == 'Invalid Date');
assertTrue(new Date('275760-10-13') == 'Invalid Date');
assertTrue(new Date('+275760-10-13 ') == 'Invalid Date');

// However, dates within the range or valid
assertTrue(new Date('100000-10-13') != 'Invalid Date');
assertTrue(new Date('+100000-10-13') != 'Invalid Date');
assertTrue(new Date('+100000-10-13 ') != 'Invalid Date');
