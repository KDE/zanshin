Feature: Available pages content
  As someone collecting tasks
  I can see all the pages available to me
  In order to display them and add tasks to them

  Scenario: Inbox, projects, contexts and tags appear in the list
    Given I display the available pages
    When I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Workday                           | go-jump-today       |
       | Projects                          | folder              |
       | Projects / Backlog                | view-pim-tasks      |
       | Projects / Prepare talk about TDD | view-pim-tasks      |
       | Projects / Read List              | view-pim-tasks      |
       | Contexts                          | folder              |
       | Contexts / Chores                 | view-pim-notes      |
       | Contexts / Internet               | view-pim-notes      |
       | Contexts / Online                 | view-pim-notes      |

