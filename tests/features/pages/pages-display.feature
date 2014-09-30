Feature: Available pages content
  As someone collecting tasks and notes
  I can see all the pages available to me
  In order to display them and add tasks or notes to them

  Scenario: Inbox, projects, contexts and topics appear in the list
    Given I display the available pages
    When I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Projects                          | folder              |
       | Projects / Read List              | view-pim-tasks      |
       | Projects / Backlog                | view-pim-tasks      |
       | Projects / Prepare talk about TDD | view-pim-tasks      |

