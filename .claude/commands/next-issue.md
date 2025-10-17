---
argumant-hint: [project-issue]
description: Selects the next issue to be implemented from a predefined implementation order and delegates the implementation to a subagent.
---
In this repo's issue #$1's body, you will find an 'implementation order' section.
From this implementation order, you will select the first not closed issue and:
  * read the issue using gh cli
  * ensure all dependent issues are closed
  * create a local branch named after the issue
  * delegate the implementation of the issue to a subagent
  * iterate with the subagent until the implementation meets all the issues requirements
  * create and submit a pull request for the issue implementation
