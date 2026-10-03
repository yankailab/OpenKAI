import test from 'node:test';
import { testCases } from './unit-tests.js';
for (const { name, run } of testCases) test(name, run);
