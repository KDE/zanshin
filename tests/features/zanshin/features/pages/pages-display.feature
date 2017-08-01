Feature: Available pages content
  As someone collecting tasks
  I can see all the pages available to me
  In order to display them and add tasks to them

  Scenario: Inbox, projects, contexts and tags appear in the list
    Given I display the available pages
    When I list the items
    Then the list is:
       | display                                       | icon                |
       | Inbox                                         | mail-folder-inbox   |
       | Workday                                       | go-jump-today       |
       | Projects                                      | folder              |
       | Projects / Calendar1                          | folder              |
       | Projects / Calendar1 / Prepare talk about TDD | view-pim-tasks      |
       | Projects / Calendar1 / Read List              | view-pim-tasks      |
       | Projects / Calendar2                          | folder              |
       | Projects / Calendar2 / Backlog                | view-pim-tasks      |
       | Contexts                                      | folder              |
       | Contexts / Errands                            | view-pim-notes      |
       | Contexts / Online                             | view-pim-notes      |

