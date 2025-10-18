---
description: Implements an issue from the backlog
argument-hint: [issue-id]
---
you will, in this order:
1. use gh cli to read the issue #$1
2. ensure all issues required before working on this one are resolved
  2.1 if not, suggest the first unresolved dependency issue to be worked on first and stop here.
3. check if the issue has a size label.
  3.1 if not, assess the complexity and add one of the following labels:
    * 'size: xs' - Less than half a day
    * 'size: s' - Half a day
    * 'size: m' - One day
    * 'size: l' - Two days (broken into subtasks)
    * 'size: xl' - More than two days (broken into subtasks)
4. if the size is 'l' or 'xl', check if the issue is already broken into sub-tasks.
  4.1 if not, break the issue into smaller sub-tasks and create new github issues for each sub-task, linking them to the original issue, advise to work on the first sub-task to be worked on and stop here.
  4.2 if yes, advise to work on the first sub-task to be worked on and stop here.
5. if the size is 'xs', 's' or 'm', create a branch named after the issue's id and title
6. delegate the implementation to a subagent
7. iterate with the subagent until the issue's requirements are met
8. create a pull request for the implementation
